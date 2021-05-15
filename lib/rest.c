#include <curl/curl.h>
#include <curl/easy.h>
#include <jansson.h>
#include <libdiscord/rest.h>
#include <libdiscord/types.h>

void create_message(bot_t *bot, const char *msg, const char *channel_id) {

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

    char *msg_data = json_dumps(payload, 0);
    json_decref(payload);

    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, msg_data);

    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
      printf("curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    }
    curl_easy_cleanup(curl);
    curl_global_cleanup();
    curl_slist_free_all(headers);

    free(auth_header);
    free(url);
  }
}