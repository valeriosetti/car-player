#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "Si468x_platform.h"
#include "Si468x_shell_cmds.h"
#include "utils.h"

void print_commands()
{
    int i;
    PRINT("Available commands:");
    for (i = 0; i < shell_commands_count; i++) {
        PRINT("%s", shell_commands[i].string);
    }
}

int main(int argc, char *argv[])
{
    int i, ret, command_ret, command_found = 0;

    if (argc < 2) {
        ERROR("specifiy at least 1 command");
        print_commands();
        return 1;
    }

    ret = Si468x_platform_init();
    if (ret)
        return ret;

    for (i = 0; i < shell_commands_count; i++) {
        if (strcmp(shell_commands[i].string, argv[1]) == 0) {
            command_found = 1;
            command_ret = shell_commands[i].func(argc - 1, &argv[1]);
        }
    }

    ret = Si468x_platform_deinit();
    if (ret)
        return ret;

    if (command_ret)
        return command_ret;

    if (!command_found) {
        ERROR("Unknown command");
        print_commands();
        return 1;
    }

    return 0;
}
