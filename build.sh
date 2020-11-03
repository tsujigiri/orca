#!/bin/bash

clang-format -i toy.c

rm -f ./toy

# GNU/Linux
cc -std=c89 -DDEBUG -Wall -Wpedantic -Wshadow -Wextra -Werror=implicit-int -Werror=incompatible-pointer-types -Werror=int-conversion -Wvla -g -Og -fsanitize=address -fsanitize=undefined -o toy toy.c

# Plan9
# pcc toy.c -o toy

./toy demo.orca
