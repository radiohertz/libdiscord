#include <libdiscord/gateway.h>
#include <libdiscord/wsc.h>
#include <libdiscord/bot.h>
#include <libdiscord/rest.h>
#include <libdiscord/types.h>
#include <string.h>
#include <sys/time.h>

#define TOKEN "NzQzNzE3MzA4NTM4ODE0NDY1.XzYutw.TeRF38v_cWzjhpHziY5htEecy2Y"

void msg(bot_t *bot, message_t msg) {

  if (strcmp(msg.content, "$ping") == 0) {

    create_message(bot, "Pong!", msg.channel_id, NULL, NULL);
  } else if (strstr(msg.content, "nice") != NULL) {
    create_message(bot, "( ͡° ͜ʖ ͡°)", msg.channel_id, NULL, &msg);

  } else if (strcmp(msg.content, "makemd") == 0) {
    embed_t *emd =
        make_embed("Hello world",
                   "Radiohead are an English rock band formed in Abingdon, "
                   "Oxfordshire, in 1985. The band consists of Thom Yorke, "
                   "brothers Jonny Greenwood and Colin Greenwood, Ed O'Brien "
                   "and Philip Selway. They have worked with producer Nigel "
                   "Godrich and cover artist Stanley Donwood since 1994",
                   "rich", NULL, NULL, 0x254873);
    create_message(bot, "Wow, an embed!!", msg.channel_id, emd, NULL);
    free_embed(emd);

  } else if (strstr(msg.content, "setpr") != NULL) {

    presence_t pr = {};
    pr.afk = false;
    pr.since = 0;
    pr.status = "online";

    strtok(msg.content, " ");
    char *name = strtok(NULL, " ");

    pr.activities.name = name;
    pr.activities.type = 2;

    set_activity(bot, &pr);
  }
}

int main() {

  // const char *token = getenv("TOKEN");

  bot_t *bot = make_bot(TOKEN, GUILD_MESSAGES);
  bot->on_msg = msg;

  run(bot);

  free_bot(bot);
}