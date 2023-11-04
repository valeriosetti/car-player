#include "Si468x.h"
#include "Si468x_ext.h"
#include "Si468x_shell_cmds.h"
#include <stdio.h>
#include "stdlib.h"
#include "string.h"
#include <unistd.h>
#include "utils.h"

COMMANDS* tuner_commands = NULL;
int tuner_commands_length = 0;

void print_commands()
{
    int i;
    printf("===============================================\n");
    printf("Available commands:\n");
    for (i=0; i<tuner_commands_length; i++) {
        printf("(%d) %s\n", i, tuner_commands[i].string);
    }
    printf("> ");
}

int main(int argc, char *argv[])
{
	int input_command_code;
	
	if (start_tuner(&tuner_commands, &tuner_commands_length) < 0) {
		return -1;
	}
	
	while(1) {
		setbuf(stdout, NULL);
		print_commands();
		if (get_int_from_stdin(0, tuner_commands_length, INPUT_AS_DEC, &input_command_code) < 0) {
			continue;
		}
		printf("Selected command: %d\n", input_command_code);
		tuner_commands[input_command_code].func();
	}
	
    stop_tuner();
    return 0;
}
