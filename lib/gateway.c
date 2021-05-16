#define _GNU_SOURCE
#include "libdiscord/types.h"
#include "libdiscord/wsc.h"
#include "stdio.h"
#include "string.h"
#include <jansson.h>
#include <libdiscord/bot.h>
#include <libdiscord/gateway.h>
#include <pthread.h>
#include <unistd.h>

bot_t *make_bot(const char *token, int intents) {
  bot_t *bot = malloc(sizeof(bot_t));
  size_t token_len = strlen(token);

  bot->token = malloc(sizeof(char) * token_len);
  strncpy(bot->token, token, token_len);

  bot->intents = intents;
  return bot;
}

void free_bot(bot_t *bot) {
  if (!bot)
    return;

  if (bot->token)
    free(bot->token);

  if (bot->_conn)
    ws_close(bot->_conn);

  free(bot);
}

typedef struct {
  ws_conn_t *conn;
  pthread_mutex_t *mut;
  int heartbeat_interval;
  int s;
} shared_state;

void *heartbeat_thread(void *shared_data) {

  shared_state *state = shared_data;

  while (1) {
    sleep(state->heartbeat_interval / 1000);

    json_t *hb_json = json_object();
    json_object_set_new(hb_json, "op", json_integer(1));
    if (!state->s)
      json_object_set_new(hb_json, "d", json_null());
    else
      json_object_set_new(hb_json, "d", json_integer(state->s));

    char *payload = json_dumps(hb_json, 0);
    json_decref(hb_json);

    send_frame(state->conn, TEXT, payload, strlen(payload));
    free(payload);
  }
}

message_t parse_message(json_t *d) {
  message_t msg = {};

  asprintf(&msg.id, "%s", json_string_value(json_object_get(d, "id")));
  asprintf(&msg.channel_id, "%s",
           json_string_value(json_object_get(d, "channel_id")));

  json_t *guild_j = json_object_get(d, "guild_id");
  if (!json_is_null(guild_j)) {
    asprintf(&msg.guild_id, "%s", json_string_value(guild_j));
  }
  json_decref(guild_j);

  asprintf(&msg.content, "%s",
           json_string_value(json_object_get(d, "content")));

  asprintf(&msg.timestamp, "%s",
           json_string_value(json_object_get(d, "timestamp")));

  msg.tts = json_boolean_value(json_object_get(d, "tts"));
  msg.mention_everyone =
      json_boolean_value(json_object_get(d, "mention_everyone"));
  msg.pinned = json_boolean_value(json_object_get(d, "pinned"));
  msg.type = json_integer_value(json_object_get(d, "type"));

  json_decref(d);
  return msg;
}

void free_message_t(message_t *msg) {
  if (msg->channel_id)
    free(msg->channel_id);
  if (msg->content)
    free(msg->content);
  if (msg->guild_id)
    free(msg->guild_id);
  if (msg->id)
    free(msg->id);
  if (msg->timestamp)
    free(msg->timestamp);
}

void run(bot_t *bot) {

  ws_conn_t conn = {.uri = "gateway.discord.gg", .port = "443"};
  conn.debug = true;

  bot->_conn = &conn;

  handshake(&conn, "/?v=9&encoding=json");

  ws_frame *hello_frame = read_frame(&conn);

  json_error_t ec;
  json_t *root = json_loads(hello_frame->payload, 0, &ec);
  free_frame(hello_frame);
  if (!root) {
    printf("Err: %d - %s\n", ec.line, ec.text);
    exit(-1);
  }

  int hb = json_integer_value(
      json_object_get(json_object_get(root, "d"), "heartbeat_interval"));

  json_decref(root);

  shared_state s = {.heartbeat_interval = hb, .conn = &conn};

  pthread_t hb_t;
  pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

  s.mut = &mutex;

  pthread_create(&hb_t, NULL, heartbeat_thread, &s);

  root = json_object();

  json_object_set_new(root, "token", json_string(bot->token));
  json_object_set_new(root, "intents", json_integer(bot->intents));

  json_t *props = json_object();

#ifdef __linux__
  json_object_set_new(props, "$os", json_string("linux"));
#elif _WIN64
  json_object_set_new(props, "$os", json_string("windows"));
#endif

  json_object_set_new(props, "$device", json_string("libdiscord"));
  json_object_set_new(props, "$browser", json_string("libdiscord"));

  json_object_set(root, "properties", props);

  json_t *identify = json_object();

  json_object_set_new(identify, "op", json_integer(2));
  json_object_set_new(identify, "d", root);

  char *identify_payload = json_dumps(identify, 0);

  printf("Identify json: %s\n", identify_payload);

  json_decref(identify);
  json_decref(props);
  json_decref(root);

  send_frame(&conn, TEXT, identify_payload, strlen(identify_payload));
  free(identify_payload);
  ws_frame *ready_frame = read_frame(&conn);

  // TOO lazy to parse the ready frame.
  // Send in a PR pls.
  free_frame(ready_frame);

  while (1) {
    ws_frame *frame = read_frame(&conn);

    json_t *root = json_loads(frame->payload, 0, &ec);
    // free_frame(frame);
    if (!root) {
      printf("Failed parsing json: %d - %s\n", ec.line, ec.text);
      continue;
    }

    json_t *event_type = json_object_get(root, "t");
    bool is_null = json_is_null(event_type);

    int op = json_integer_value(json_object_get(root, "op"));
    int seq = json_integer_value(json_object_get(root, "s"));

    if (is_null && op == 11) {
      json_decref(event_type);
      continue;
    }
    s.s = seq;

    const char *type = json_string_value(event_type);

    if (strcmp(type, "MESSAGE_CREATE") == 0) {
      message_t msg = parse_message(json_object_get(root, "d"));
      if (bot->on_msg) {
        bot->on_msg(bot, msg);
      }
      free_message_t(&msg);
    } else if (strcmp(type, "MESSAGE_UPDATE") == 0) {
    } else {

      printf("unimplemented event: %s\n", frame->payload);
    }

    json_decref(event_type);
    json_decref(root);
  }

  ws_close(&conn);
}

void set_activity(bot_t *bot, presence_t *presence) {
  if (!presence)
    return;

  json_t *root = json_object();
  json_object_set_new(root, "op", json_integer(3));

  json_t *d = json_object();
  if (presence->since)
    json_object_set_new(d, "since", json_integer(presence->since));
  else
    json_object_set_new(d, "since", json_null());

  json_object_set_new(d, "afk", json_boolean(presence->afk));
  json_object_set_new(d, "status", json_string(presence->status));

  json_t *acts = NULL;
  json_t *act = NULL;
  acts = json_array();

  act = json_object();

  json_object_set_new(act, "name", json_string(presence->activities.name));
  json_object_set_new(act, "type", json_integer(presence->activities.type));

  json_object_set(d, "activities", acts);
  json_array_append(acts, act);

  json_object_set(root, "d", d);

  char *payload = json_dumps(root, 0);
  printf("%s\n", payload);

  if (act)
    json_decref(act);

  if (acts)
    json_decref(acts);

  json_decref(d);
  json_decref(root);

  printf("Data: %s\n", payload);

  send_frame(bot->_conn, TEXT, payload, strlen(payload));
  free(payload);
}