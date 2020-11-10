#pragma once

#include <stdio.h>

#define MAXSZ 128 * 128

typedef struct Grid {
	int w, h, l, f, r;
	int lock[MAXSZ];
	int type[MAXSZ];
	char vars[36];
	char data[MAXSZ];
} Grid;

void print(Grid *g);
int run(Grid *g);
int disk(FILE *f, Grid *g);
