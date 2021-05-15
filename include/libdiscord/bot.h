#ifndef _LIBDISCORD_BOT_H
#define _LIBDISCORD_BOT_H
#include "types.h"

// Create a new bot instance.
bot_t *make_bot(const char *token, int intents);

// free the bot.
void free_bot(bot_t *bot);

// Start the bot.
void run(bot_t *bot);

#endif