#!/bin/sh
set -e

rm -f ./out/app
mkdir -p ./out
g++ -g -O0 -Wall -Wextra -DLINUX -DHANDMADE_SLOW=1 -DHANDMADE_INTERNAL=1 -o ./out/app ./src/LINUX_main.cpp $(pkg-config --cflags --libs sdl2) -lm
