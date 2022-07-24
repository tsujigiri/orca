#!/bin/sh -e

echo "Cleaning.."
rm -rf bin
mkdir bin

if [ -e "$HOME/roms/uxnlin.rom" ]
then
	echo "Linting.."
	uxncli $HOME/roms/uxnlin.rom src/orca.tal
	uxncli $HOME/roms/uxnlin.rom src/library.tal
	uxncli $HOME/roms/uxnlin.rom src/manifest.tal
fi

echo "Assembling.."
uxnasm src/orca.tal bin/orca.rom 

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
