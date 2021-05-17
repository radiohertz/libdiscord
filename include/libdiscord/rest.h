#ifndef _LIBDISCORD_REST_H
#define _LIBDISCORD_REST_H
#include "types.h"

// const char BASE_API_URL[] = "https://discord.com/api/v9";

// Create a new message in a guild.
//
// The first three parameters should be non NULL.
//
// `emd` should be NULL if the message is not a embed.
//
// `reply_to` should be NULL if the message is not a reply.
void create_message(bot_t *bot, const char *msg, const char *channel_id,
                    embed_t *emd, message_t *reply_to);

/// Make a embed that can be used in `create_message`.
///
/// If the field is not required, pass in NULL for pointers and 0 for int.
embed_t *make_embed(const char *title, const char *description,
                    const char *type, const char *url, const char *timestamp,
                    int color);

void free_embed(embed_t *emd);

#endif