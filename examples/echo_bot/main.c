#include "libdiscord/rest.h"
#include "libdiscord/types.h"
#include <libdiscord/bot.h>
#include <string.h>

void msg(bot_t *bot, message_t msg) {
  if (strcmp(msg.content, "$ping") == 0) {
    printf("Got : %s from channel: %s\n", msg.content, msg.channel_id);
    create_message(bot, "pong!", msg.channel_id);
  } else if (strstr(msg.content, "nice") != NULL) {
    create_message(bot, "( ͡° ͜ʖ ͡°)", msg.channel_id);
  }
}

int main() {

  const char *token = getenv("TOKEN");

  bot_t *bot = make_bot(token, GUILD_MESSAGES);
  bot->on_msg = msg;

  run(bot);

  free_bot(bot);
}