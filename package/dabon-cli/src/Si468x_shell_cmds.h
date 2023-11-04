#ifndef _SI468X_SHELL_CMDS_H_
#define _SI468X_SHELL_CMDS_H_

#include "stdint.h"

enum tuner_fw {
	DAB_FW,
	FM_FW
};

typedef struct {
    char* string;
    int (*func)(void);
} COMMANDS;

int start_tuner(COMMANDS** tuner_commands, int* tuner_commands_length);

#endif //_SI468X_SHELL_CMDS_H_
