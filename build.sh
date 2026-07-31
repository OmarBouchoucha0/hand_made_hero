#!/bin/sh
set -e

mkdir -p ./out
g++ -g -O0 -Wall -Wextra -o ./out/app ./src/LINUX_main.cpp $(pkg-config --cflags --libs sdl2) -lm
