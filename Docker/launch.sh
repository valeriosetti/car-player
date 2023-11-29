#!/bin/bash

COMMAND=${1:-}

if [ "$COMMAND" != "" ]; then
    # Start docker and launch the specified command (no interactive shell)
    docker run --rm -v$(pwd)/..:/home/builder --user builder --workdir /home/builder buildroot:01.00 $COMMAND
else
    # Start docker with interactive shel
    docker run -it --rm -v$(pwd)/..:/home/builder --user builder --workdir /home/builder buildroot:01.00
fi
