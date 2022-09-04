#!/bin/sh -e

# ~/Applications/butler push bin/orca.rom hundredrabbits/orca:uxn

ASM="uxncli $HOME/roms/drifblim.rom"
EMU="uxnemu"
LIN="uxncli $HOME/roms/uxnlin.rom"

SRC="src/orca.tal"
DST="bin/orca.rom"

CPY="$HOME/roms"
ETC="src/manifest.tal src/library"
ARG="etc/tests.orca"

echo ">> Cleaning"
rm -rf bin
mkdir bin

if [[ "$*" == *"--lint"* ]]
then
    echo ">> Linting $SRC"
	$LIN $SRC $ETC
fi

echo ">> Assembling $SRC"
$ASM $SRC $DST

if [[ "$*" == *"--save"* ]]
then
    echo ">> Saving $DST"
	cp $DST $CPY
fi

echo ">> Running $DST"
$EMU $DST $ARG
