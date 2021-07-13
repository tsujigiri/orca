#!/bin/bash

echo "Cleaning.."
rm -rf bin
mkdir bin

echo "Assembling.."
cat src/main.tal src/assets.tal > src/orca.tal
uxnasm src/orca.tal bin/orca.rom 
rm src/orca.tal

echo "Installing.."
if [ -d "$HOME/roms" ] && [ -e ./bin/orca.rom ]
then
	cp ./bin/orca.rom $HOME/roms
    echo "Installed in $HOME/roms" 
fi

if [ "${1}" = '--push' ]; 
then
	echo "Pushing.."
	~/Applications/butler push bin/orca.rom hundredrabbits/orca:uxn
fi

echo "Running.."
uxnemu bin/orca.rom
