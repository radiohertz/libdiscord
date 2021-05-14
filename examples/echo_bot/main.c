#include "libdiscord/types.h"
#include <libdiscord/bot.h>

int main() {

  const char *token = getenv("TOKEN");

  bot_t *bot = make_bot(token, GUILD_MESSAGES);

  run(bot);

  free_bot(bot);
}