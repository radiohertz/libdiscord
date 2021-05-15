#ifndef _LIBDISCORD_TYPES_H
#define _LIBDISCORD_TYPES_H
#include "wsc.h"

typedef struct bot_t bot_t;

typedef struct {
  char *id;
  char *channel_id;
  char *guild_id;
  // user_t user;
  char *content;
  char *timestamp;
  bool tts;
  bool mention_everyone;
  bool pinned;
  int type;

} message_t;

typedef void(on_message)(bot_t *bot, message_t msg);

typedef struct bot_t {
  char *token;
  ws_conn_t *_conn;
  int intents;
  on_message *on_msg;
} bot_t;

typedef struct {
  char *id;
  char *username;
  char *discriminator;
  char *avatar;
  bool bot;
  bool system;
  bool mfa_enabled;
  bool locale;
  bool verified;
  char *email;
  int flags;
  uint8_t premium_type;
  int public_flags;
} user_t;

typedef struct {
  char *id;
  bool unavailable;
} unavailable_guild_t;

typedef struct {
  char *id;
  int flags;
} application_t;

typedef struct {
  int v;
  user_t user;
  unavailable_guild_t *guilds;
  char *session_id;
  int shard[2];
  application_t application;
} ready_t;

typedef enum {

  GUILDS = 1 << 0,
  GUILD_MEMBERS = 1 << 1,
  GUILD_BANS = 1 << 2,
  GUILD_EMOJIS = 1 << 3,
  GUILD_INTEGRATIONS = 1 << 4,
  GUILD_WEBHOOKS = 1 << 5,
  GUILD_INVITES = 1 << 6,
  GUILD_VOICE_STATES = 1 << 7,
  GUILD_PRESENCES = 1 << 8,
  GUILD_MESSAGES = 1 << 9,
  GUILD_MESSAGE_REACTIONS = 1 << 10,
  GUILD_MESSAGE_TYPING = 1 << 11,
  DIRECT_MESSAGES = 1 << 12,
  DIRECT_MESSAGE_REACTIONS = 1 << 13,
  DIRECT_MESSAGE_TYPING = 1 << 14
} gateway_intents;

#endif