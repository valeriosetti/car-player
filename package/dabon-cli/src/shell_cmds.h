#ifndef _SI468X_SHELL_CMDS_H_
#define _SI468X_SHELL_CMDS_H_

#include "stdint.h"

enum tuner_fw {
    DAB_FW,
    FM_FW
};

typedef struct {
    char* string;
    int (*func)(int argc, char *argv[]);
} SHELL_COMMAND;

extern SHELL_COMMAND shell_commands[];
extern int shell_commands_count;

#endif //_SI468X_SHELL_CMDS_H_
