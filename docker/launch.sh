#!/bin/bash

COMMAND=${1:-}
SCRIPT_PATH=$(dirname $(readlink -f "$0"))
PRJ_ROOT=$SCRIPT_PATH/..

if [ "$COMMAND" != "" ]; then
    # Start docker and launch the specified command (no interactive shell)
    docker run --rm -v$PRJ_ROOT:/home/br-user --user $(id -u) buildroot:latest $COMMAND
else
    # Start docker with interactive shel
    docker run -it --rm -v$PRJ_ROOT:/home/br-user --user $(id -u) buildroot:latest
fi
