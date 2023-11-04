#!/bin/bash

COMMAND=${1:-}

docker run -it --rm -v$(pwd)/..:/home/builder --user builder --workdir /home/builder buildroot:01.00 $COMMAND
