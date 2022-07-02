#!/bin/bash

# ************************
# Some handy stuff
# ************************
RED='\033[0;31m'
BOLDRED='\033[1;31m'
GREEN='\033[0;32m'
BLUE='\033[0;34m'
BOLDBLUE='\033[1;34m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

function Message()
{
	if [ -z "$2" ]; then
        echo -e "$GREEN$1$NC"
    else
        echo -e "$1$2$NC"
    fi
}

# ************************
# Our code
# ************************

cmake -S . -B build/debug -DCMAKE_BUILD_TYPE=Debug
cmake -S . -B build/release -DCMAKE_BUILD_TYPE=Release

cmake --build build/debug --target EdgeUI.X11 -- -j8
cmake --build build/release --target EdgeUI.X11 -- -j8

cmake --build build/debug --target EdgeUI.DRM -- -j8
cmake --build build/release --target EdgeUI.DRM -- -j8
