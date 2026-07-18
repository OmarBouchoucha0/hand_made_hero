#!/bin/sh

gcc -g -O0 -o ./out/app main.c -lglfw -lGL
./out/app
