#!/bin/sh -e

ASM="uxncli $HOME/roms/drifblim.rom"
EMU="uxnemu"
LIN="uxncli $HOME/roms/uxnlin.rom"

SRC="src/orca.tal"
DST="bin/orca.rom"

CPY="$HOME/roms"
ARG="etc/tests.orca"

STORE="$HOME/Applications/butler push"
STOREID="hundredrabbits/orca:uxn"

echo ">> Cleaning"
rm -rf bin
mkdir bin

if [[ "$*" == *"--lint"* ]]
then
    echo ">> Linting $SRC"
	$LIN $SRC
fi

echo ">> Assembling $SRC"
$ASM $SRC $DST

if [[ "$*" == *"--save"* ]]
then
    echo ">> Saving $DST"
	cp $DST $CPY
fi

if [[ "$*" == *"--push"* ]]
then
    echo ">> Pushing $DST"
	$STORE $DST $STOREID
fi

echo ">> Running $DST"
$EMU $DST $ARG
