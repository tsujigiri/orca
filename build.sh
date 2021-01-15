#!/bin/bash

clang-format -i orca.c

## Cleanup
rm -f ./orca

# client
cc -std=c89 -DDEBUG -Wall -Wno-unknown-pragmas -Wpedantic -Wshadow -Wextra -Werror=implicit-int -Werror=incompatible-pointer-types -Werror=int-conversion -Wvla -g -Og -fsanitize=address -fsanitize=undefined -L/usr/local/lib -lSDL2 -lportmidi orca.c -o orca
# cc -std=c89 -O2 -DNDEBUG -g0 -s -Wall -Wno-unknown-pragmas -L/usr/local/lib -lSDL2 -lportmidi orca.c -o orca

# Size
echo "Size: $(du -sk ./orca)"

# Install
if [ -d "$HOME/bin" ] && [ -e ./orca ]
then
	cp ./orca $HOME/bin
    echo "Installed: $HOME/bin" 
fi

./orca demo.orca
