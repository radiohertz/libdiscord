#include <curl/curl.h>
#include <curl/easy.h>
#include <jansson.h>
#include <libdiscord/rest.h>
#include <libdiscord/types.h>

embed_t *make_embed(const char *title, const char *description,
                    const char *type, const char *url, const char *timestamp,
                    int color) {
  embed_t *emd = malloc(sizeof(embed_t));
  *emd = (embed_t){};

  if (title)
    asprintf(&emd->title, "%s", title);
  if (description)
    asprintf(&emd->description, "%s", description);
  if (type)
    asprintf(&emd->type, "%s", type);
  if (url)
    asprintf(&emd->url, "%s", url);
  if (timestamp)
    asprintf(&emd->timestamp, "%s", timestamp);

  emd->color = color;

  return emd;
}

void free_embed(embed_t *emd) {

  if (!emd)
    return;

  if (emd->description)
    free(emd->description);
  if (emd->timestamp)
    free(emd->timestamp);
  if (emd->title)
    free(emd->title);
  if (emd->type)
    free(emd->type);
  if (emd->url)
    free(emd->url);

  free(emd);
  emd = NULL;
}

void create_message(bot_t *bot, const char *msg, const char *channel_id,
                    embed_t *emd) {

  CURL *curl;
  CURLcode res;

  curl_global_init(CURL_GLOBAL_ALL);

  curl = curl_easy_init();
  if (curl) {

    struct curl_slist *headers = NULL;

    char *auth_header;
    asprintf(&auth_header, "Authorization: Bot %s", bot->token);
    headers = curl_slist_append(headers, auth_header);
    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    char *url;
    asprintf(&url, "%s/channels/%s/messages", "https://discord.com/api/v9",
             channel_id);
    curl_easy_setopt(curl, CURLOPT_URL, url);

    json_t *payload = json_object();
    json_object_set_new(payload, "content", json_string(msg));
    json_object_set_new(payload, "tts", json_false());

    json_t *emb_json = NULL;

    if (emd) {
      emb_json = json_object();
      printf("Here\n");

      if (emd->title)
        json_object_set_new(emb_json, "title", json_string(emd->title));
      if (emd->type)
        json_object_set_new(emb_json, "type", json_string(emd->type));
      if (emd->description)
        json_object_set_new(emb_json, "description",
                            json_string(emd->description));
      if (emd->url)
        json_object_set_new(emb_json, "url", json_string(emd->url));
      if (emd->timestamp)
        json_object_set_new(emb_json, "url", json_string(emd->timestamp));

      if (emd->color)
        json_object_set_new(emb_json, "color", json_integer(emd->color));

      json_object_set(payload, "embed", emb_json);

      free_embed(emd);
    }

    char *msg_data = json_dumps(payload, 0);
    if (emb_json)
      json_decref(emb_json);
    json_decref(payload);

    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, msg_data);

    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
      printf("curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    }
    curl_easy_cleanup(curl);
    curl_global_cleanup();
    curl_slist_free_all(headers);

    free(msg_data);
    free(auth_header);
    free(url);
  }
}