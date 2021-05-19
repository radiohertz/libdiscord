/* Libdiscord and other includes */
#include <libdiscord/gateway.h>
#include <libdiscord/wsc.h>
#include <libdiscord/bot.h>
#include <libdiscord/rest.h>
#include <libdiscord/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <regex.h>

/* Token for bot */
#define TOKEN "NzQzNzE3MzA4NTM4ODE0NDY1.XzYutw.TeRF38v_cWzjhpHziY5htEecy2Y"

int main() {
	int status;
	regex_t regex;
	status = regcomp(&regex, "\\!.*",0);
	printf("%d\n",status);
	char msg[20] = "!ping msg";
	int status_exec;
	status_exec = regexec(&regex, msg, 0, NULL, 0);
	printf("%d\n",status_exec);

	return 0;
}
