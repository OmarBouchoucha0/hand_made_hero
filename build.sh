#!/bin/sh
set -e

mkdir -p ./out
mkdir -p ./assets

g++ -g -O0 -Wall -Wextra -shared -fPIC -DHANDMADE_SLOW=1 -DHANDMADE_INTERNAL=1 -o \
    ./out/handmade_temp.so ./src/handmade.cpp $(pkg-config --cflags --libs sdl2) -lm

mv ./out/handmade_temp.so ./out/handmade.so

g++ -g -O0 -march=native -Wall -Wextra -Wshadow -Wconversion -Wcast-align -Werror \
    -Wno-unused-function -Wno-unused-variable -fno-exceptions \
    -DLINUX -DHANDMADE_SLOW=1 -DHANDMADE_INTERNAL=1 \
    -o ./out/app ./src/LINUX_main.cpp $(pkg-config --cflags --libs sdl2) -lm \
    -static-libgcc -static-libstdc++ -ldl
