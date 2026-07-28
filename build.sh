#!/bin/sh
set -e

mkdir -p ./out
gcc -g -O0 -Wall -Wextra -o ./out/app ./src/main.c $(pkg-config --cflags --libs sdl2) -lm
