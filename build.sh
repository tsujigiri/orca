#!/bin/bash

clang-format -i sim.c
clang-format -i sim.h
clang-format -i cli.c
clang-format -i toy.c
clang-format -i midi.c
clang-format -i midi.h

# cli
# rm -f ./cli
# cc -std=c89 -DDEBUG -Wall -Wpedantic -Wshadow -Wextra -Werror=implicit-int -Werror=incompatible-pointer-types -Werror=int-conversion -Wvla -g -Og -fsanitize=address -fsanitize=undefined -o cli cli.c sim.c
# cc cli.c sim.c -std=c89 -O2 -DNDEBUG -g0 -s -Wall -o cli
# ./cli demo.orca

# toy
rm -f ./toy
cc -std=c89 -DDEBUG -Wall -Wpedantic -Wshadow -Wextra -Werror=implicit-int -Werror=incompatible-pointer-types -Werror=int-conversion -Wvla -g -Og -fsanitize=address -fsanitize=undefined -L/usr/local/lib -lSDL2 toy.c sim.c midi.c -o toy
# cc toy.c sim.c -std=c89 -O2 -DNDEBUG -g0 -s -Wall -L/usr/local/lib -lSDL2 -o toy
./toy
