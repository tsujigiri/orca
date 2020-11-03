#include <stdio.h>

typedef struct Grid {
	int w, h, l;
	int lock[256 * 256];
	char data[256 * 256];
} Grid;

int
error(char *name)
{
	printf("Error: %s\n", name);
	return 0;
}

void
lock(Grid *g, int x, int y, int b)
{
	g->lock[x + (y * g->w)] = b;
}

int
busy(Grid *g, int x, int y)
{
	return g->lock[x + (y * g->w)];
}

char
get(Grid *g, int x, int y)
{
	return g->data[x + (y * g->w)];
}

void
set(Grid *g, int x, int y, char c)
{
	g->data[x + (y * g->w)] = c;
}

int
getint(Grid *g, int x, int y)
{
	return get(g, x, y) - '0';
}

void
parse(Grid *g)
{
	int i, x, y;
	printf("F0\n");
	for(i = 0; i < g->l; ++i) {
		printf("%c", g->data[i]);
	}
	printf("DEBUG\n");
	for(i = 0; i < g->l; ++i) {
		char c = g->data[i];
		x = i % g->w;
		y = i / g->w;
		if(busy(g, x, y))
			continue;
		if(c == 'A') {
			set(g, x, y + 1, '0');
		} else if(c == 'E') {
			if(x == g->w || get(g, x + 1, y) != '.')
				set(g, x, y, '*');
			else {
				set(g, x, y, '.');
				set(g, x + 1, y, 'E');
				lock(g, x + 1, y, 1);
			}
		} else if(c == 'N') {
			if(y == 0 || get(g, x, y - 1) != '.')
				set(g, x, y, '*');
			else {
				set(g, x, y, '.');
				set(g, x, y - 1, 'N');
				lock(g, x, y - 1, 1);
			}
		} else if(c == 'S') {
			if(y == g->h || get(g, x, y + 1) != '.')
				set(g, x, y, '*');
			else {
				set(g, x, y, '.');
				set(g, x, y + 1, 'S');
				lock(g, x, y + 1, 1);
			}
		} else if(c == 'W') {
			if(x == 0 || get(g, x - 1, y) != '.')
				set(g, x, y, '*');
			else {
				set(g, x, y, '.');
				set(g, x - 1, y, 'W');
				lock(g, x - 1, y, 1);
			}
		}
	}
	printf("F1\n");
	for(i = 0; i < g->l; ++i) {
		printf("%c", g->data[i]);
	}
}

void
load(FILE *f, Grid *g)
{
	char c;
	g->l = 0;
	while((c = fgetc(f)) != EOF) {
		if(c == '\n') {
			if(g->w == 0)
				g->w = g->l + 1;
			g->h = g->l / g->w;
		}
		g->data[g->l++] = c;
	}
	printf("grid:%d(%dx%d)\n", g->l, g->w, g->h);
	parse(g);
}

int
main(int argc, char *argv[])
{
	FILE *f;
	Grid g;
	if(argc < 2)
		return error("No input.");
	f = fopen(argv[1], "r");
	if(f == NULL)
		return error("Missing input.");
	load(f, &g);
	return 0;
}
