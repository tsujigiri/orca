# Orca

Orca is an [esoteric programming language](https://wiki.xxiivv.com/orca) designed to quickly create procedural sequencers, in which every letter of the alphabet is an operation, where lowercase letters operate on bang, uppercase letters operate each frame.

This is a minimalist implementation of the basic operators created for educational purposes. To try a complete environment with client and server, see [Orca](https://git.sr.ht/~rabbits/orca).

## Build

You must have [SDL2](https://wiki.libsdl.org/) and [portmidi](http://portmedia.sourceforge.net/portmidi/).

```
cc orca.c -std=c89 -O2 -DNDEBUG -g0 -s -Wall -L/usr/local/lib -lSDL2 -o orca
```

## Operators

To display the list of operators inside of Orca, use `CmdOrCtrl+G`.

- `A` **add**(*a* b): Outputs sum of inputs.
- `B` **subtract**(*a* b): Outputs difference of inputs.
- `C` **clock**(*rate* mod): Outputs modulo of frame.
- `D` **delay**(*rate* mod): Bangs on modulo of frame.
- `E` **east**: Moves eastward, or bangs.
- `F` **if**(*a* b): Bangs if inputs are equal.
- `G` **generator**(*x* *y* *len*): Writes operands with offset.
- `H` **halt**: Halts southward operand.
- `I` **increment**(*step* mod): Increments southward operand.
- `J` **jumper**(*val*): Outputs northward operand.
- `K` **konkat**(*len*): Reads multiple variables.
- `L` **less**(*a* *b*): Outputs smallest of inputs.
- `M` **multiply**(*a* b): Outputs product of inputs.
- `N` **north**: Moves Northward, or bangs.
- `O` **read**(*x* *y* read): Reads operand with offset.
- `P` **push**(*len* *key* val): Writes eastward operand.
- `Q` **query**(*x* *y* *len*): Reads operands with offset.
- `R` **random**(*min* max): Outputs random value.
- `S` **south**: Moves southward, or bangs.
- `T` **track**(*key* *len* val): Reads eastward operand.
- `U` **uclid**(*step* max): Bangs on Euclidean rhythm.
- `V` **variable**(*write* read): Reads and writes variable.
- `W` **west**: Moves westward, or bangs.
- `X` **write**(*x* *y* val): Writes operand with offset.
- `Y` **jymper**(*val*): Outputs westward operand.
- `Z` **lerp**(*rate* target): Transitions operand to input.
- `*` **bang**: Bangs neighboring operands.
- `#` **comment**: Halts a line.

## Controls

- `PAGEUP`, incr. bpm.
- `PAGEDOWN`, decr. bpm.
- `+` zoom in.
- `-` zoom out.
- `SHIFT+ARROW` scale selection.
- `CTRL+ARROW` drag selection.
- `ALT+ARROW` skip selection.
- `ARROW` move selection.
- `SPACE` toggle play.
- `BACKSPACE` erase.
- `CTRL+/` comment selection.
- `CTRL+U` uppercase selection.
- `CTRL+L` lowercase selection.
- `CTRL+[` decrement selection.
- `CTRL+]` increment selection.

## Syntax Highlight

- `1` locked
- `2` port(left)
- `3` operator
- `4` port(right)
- `5` port(output)

## TODOs

- True clock
	- Makeup for render time.
- Selection right-to-left drag.
- Simple Undo