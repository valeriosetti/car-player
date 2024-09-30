#!/bin/bash

set -e
clear;

if [ ! -d build ]; then
	(
		mkdir build
		cd build
		cmake ..
	)
fi

(
	cd build
	make -j32
)
