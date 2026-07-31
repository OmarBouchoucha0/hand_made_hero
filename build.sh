#!/bin/sh
set -e

mkdir -p ./out
gcc -g -O0 -Wall -Wextra -o ./out/app ./src/LINUX_main.c $(pkg-config --cflags --libs sdl2) -lm
