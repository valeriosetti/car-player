#!/bin/bash

set -e
clear;

if [ -d build ]; then
	rm -r build
fi

./build.sh
