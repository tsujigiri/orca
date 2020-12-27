#!/bin/bash

clang-format -i sim.c
clang-format -i sim.h
clang-format -i cli.c
clang-format -i orca.c

## Cleanup
rm -f ./cli
rm -f ./orca

# cli
# cc -std=c89 -DDEBUG -Wall -Wpedantic -Wshadow -Wextra -Werror=implicit-int -Werror=incompatible-pointer-types -Werror=int-conversion -Wvla -g -Og -fsanitize=address -fsanitize=undefined -o cli cli.c sim.c
# cc cli.c sim.c -std=c89 -O2 -DNDEBUG -g0 -s -Wall -o cli
# ./cli demo.orca

# client
cc -std=c89 -DDEBUG -Wall -Wpedantic -Wshadow -Wextra -Werror=implicit-int -Werror=incompatible-pointer-types -Werror=int-conversion -Wvla -g -Og -fsanitize=address -fsanitize=undefined -L/usr/local/lib -lSDL2 -lportmidi orca.c sim.c -o orca
# cc -std=c89 -O2 -DNDEBUG -g0 -s -Wall -L/usr/local/lib -lSDL2 -lportmidi orca.c sim.c -o orca

# Size
echo "Size: $(du -sk ./orca)"

# Install
if [ -d "$HOME/bin" ] && [ -e ./orca ]
then
	cp ./orca $HOME/bin
    echo "Installed: $HOME/bin" 
fi

./orca demo.orca
