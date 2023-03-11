UXNEMU ?= uxnemu
UXNCLI ?= uxncli
UXN_ROMS ?= ${HOME}/roms
UXNASM ?= ${UXNCLI} ${UXN_ROMS}/drifblim.rom
UXNLIN ?= ${UXN_ROMS}/uxnlin.rom

.SUFFIXES:
.SUFFIXES: .tal .rom

build: bin/orca.rom

lint: src/orca.tal
	${UXNCLI} ${UXNLIN} $^

install:
	cp bin/orca.rom ${UXN_ROMS}/

run: bin/orca.rom
	${UXNEMU} $^

clean:
	rm -f bin/*

bin/orca.rom: src/*.tal
	${UXNASM} src/orca.tal $@

.PHONY: build lint install run clean
