#include "libdiscord/types.h"
#include "libdiscord/wsc.h"
#include "string.h"
#include <jansson.h>
#include <libdiscord/bot.h>
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

void *heartbeat_thread(void *shared_data) {}

void run(bot_t *bot) {

  ws_conn_t conn = {.uri = "gateway.discord.gg", .port = "443"};

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

  printf("Read from : %s\n", ready_frame->payload);

  free_frame(ready_frame);

  ws_close(&conn);
}