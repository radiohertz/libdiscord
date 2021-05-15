#ifndef _LIBDISCORD_REST_H
#define _LIBDISCORD_REST_H
#include "types.h"

// const char BASE_API_URL[] = "https://discord.com/api/v9";

void create_message(bot_t *bot, const char *msg, const char *channel_id);

#endif