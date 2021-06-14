#!/bin/bash

echo "Cleaning.."
rm -rf bin
mkdir bin

echo "Assembling.."
uxnasm orca.tal bin/orca.rom 

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
