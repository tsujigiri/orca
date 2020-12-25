#pragma once

#include <stdio.h>

#define MAXMSG 64
#define MAXSZ 128 * 128

typedef struct Grid {
	int w, h, l, f, r, msglen;
	int lock[MAXSZ], type[MAXSZ];
	char data[MAXSZ], vars[36], msg[MAXMSG];
} Grid;

int base36(char c);
int cisp(char c);

char get(Grid *g, int x, int y);
void set(Grid *g, int x, int y, char c);
int gettype(Grid *g, int x, int y);

int rungrid(Grid *g);
int loadgrid(Grid *g, FILE *f);
void initgrid(Grid *g, int w, int h);
