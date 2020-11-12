#pragma once

#include <stdio.h>

#define MAXSZ 128 * 128

typedef struct Grid {
	int w, h, l, f, r;
	int lock[MAXSZ];
	int type[MAXSZ];
	char vars[36];
	char data[MAXSZ];
	char msg[36];
	int msg_len;
} Grid;

char get(Grid *g, int x, int y);
void set(Grid *g, int x, int y, char c);
int gettype(Grid *g, int x, int y);
int run(Grid *g);
int disk(FILE *f, Grid *g);
void create(Grid *g, int w, int h);
void print(Grid *g);
