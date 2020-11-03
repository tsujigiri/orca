# GYO

[Gyo](https://wiki.xxiivv.com/site/gyo.html) is an imaginary computer with 16 registers, 256 addressable memory addresses. The cpu understands 16 opcodes, the assembler syntax understands `=constants` and `:labels`. Programs are loaded directly into memory to create procedural self-modifiable programs.

<img src='https://wiki.xxiivv.com/media/generic/gyo.jpg' width='200'/>

## Instructions

```
0x0  BRK  Break
0x1  JMP  Jump to position in memory
0x2  JCA  Jump to position in memory, when carryflag
0x3  JZE  Jump to position in memory, when zeroflag
0x4  PEK  Read status
0x5  POK  Write status
0x6  GET  Store in register
0x7  PUT  Store in memory
0x8  ADD  Add value to register, set carryflag when overflow
0x9  SUB  Substract value from register, set carryflag when overflow
0xA  EQU  Compare register with value, set zeroflag when equal
0xB  LES  Compare register with value, set zeroflag when less than
0xC  AND  Bitwise AND operator
0xD  XOR  Bitwise XOR operator
0xE  ROL  Bitwise rotate left
0xF  ROR  Bitwise rotate right
```

## Addressing

- `#ef` the raw value.
- `$ef` the value at address #ef in memory.
- `[e]` the value of the 15th register.
- `{e}` the value at the address of the 15th register.

## IO

To begin IO mode, activate the traps flag with `POK #08`.

```
POK #08  ; Set traps flag ON
GET      ; Read character from stdin
PUT [0]  ; Write character to stdout
POK #00  ; Set traps flag OFF 
```

## Status Register

The status register is distributed as follows, it shares the same byte as the register selector. The **halt** flag is used by the BRK instruction is stops the cpu, the **zero** flag is used for conditional instructions(EQU/LES/JEQ/JNE), the **carry** flag is set when a SUB/ADD instruction carries over the 8bit range and the **traps** flag is set when the pointer should be sent to the traps location(in IO operations). 

```
T C Z H
| | | +---- Halt
| | +------ Zero
| +-------- Carry
+---------- Traps
```

## Assembly Syntax

```
=rate #01           ; defines constant value

:label              ; a label defines an address in the program
	ADD rate        ; add rate to register
	EQU #0f         ; set zeroflag if register is equal to #0f
	JZE end         ; goto end if equal
	JMP label       
:end
	BRK
```

If you need to fill the memory with specific values, you can do it as follows:

```
:hello
	#48 #65
	#6C #6C
	#6F #0A
```