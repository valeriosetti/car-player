#!/bin/bash

SCRIPT_PATH=$(dirname $(readlink -f "$0"))
docker build \
        -t buildroot \
        $SCRIPT_PATH/../buildroot/support/docker/
