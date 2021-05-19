/* Libdiscord and other includes */
#include <libdiscord/gateway.h>
#include <libdiscord/wsc.h>
#include <libdiscord/bot.h>
#include <libdiscord/rest.h>
#include <libdiscord/types.h>
#include <regex.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>

/* Token for bot */
#define TOKEN "Your Bot Token here(inside quotes)"

/* Regex global variable */
regex_t regex;

void msg(bot_t* bot, message_t msg) {
    
    /* Ping command */
	char command[6] = "!ping";
    if(regexec(&regex, command, 0, NULL, 0) == 0) {
    	if(strcmp(msg.content, "!ping") == 0){
        	create_message(bot, "Pong!", msg.channel_id, NULL, NULL);
    	}
    }
}

int main(){
    /* Create a new bot instance*/
    bot_t* bot = make_bot(TOKEN, GUILD_MESSAGES);

	/* regex area */
	char* prefix = "\\!.*"; // Replace the "!" with your own prefix. The same in msg function. 
    int status;
    status = regcomp(&regex, prefix, 0);
	if(status == 0) {} 
	else {goto exit;}
    
	bot->on_msg = msg;
    run(bot);
	
	exit:
    /* Free the bot instance */
    free_bot(bot);
	regfree(&regex);
    return 0;
}