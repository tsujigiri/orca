#!/bin/bash

clang-format -i toy.c

rm -f ./toy

# GNU/Linux
# cc -std=c89 -DDEBUG -Wall -Wpedantic -Wshadow -Wextra -Werror=implicit-int -Werror=incompatible-pointer-types -Werror=int-conversion -Wvla -g -Og -fsanitize=address -fsanitize=undefined -o toy toy.c

# GNU/Linux
cc toy.c -std=c89 -O2 -DNDEBUG -g0 -s -Wall -o toy

# Plan9
# pcc toy.c -o toy

# ./toy abflm.orca > abflm-result.orca
# diff abflm-result.orca abflm-expected.orca

time ./toy ~/Git/orca-examples/benchmarks/logic.orca
