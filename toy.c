#include <stdio.h>

typedef struct Grid {
	int w, h, l;
	int lock[256 * 256];
	char data[256 * 256];
} Grid;

int
cint(char c)
{
	if(c == '.')
		return 0;
	if(c >= '0' && c <= '9')
		return c - '0';
	if(c >= 'a' && c <= 'z')
		return c - 'a' + 10;
	if(c >= 'A' && c <= 'Z')
		return c - 'A' + 10;
	return 0;
}

char
cchr(int v, int cap)
{
	v %= 36;
	v *= v < 0 ? -1 : 1;
	if(v >= 0 && v <= 9)
		return '0' + v;
	if(cap)
		return 'A' + (v - 10);
	return 'a' + (v - 10);
}

int
error(char *name)
{
	printf("Error: %s\n", name);
	return 0;
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
	return cint(get(g, x, y));
}

void
setint(Grid *g, int x, int y, int v)
{
	g->data[x + (y * g->w)] = cchr(v, 0);
}

void
lock(Grid *g, int x, int y)
{
	g->lock[x + (y * g->w)] = 1;
}

int
busy(Grid *g, int x, int y)
{
	return g->lock[x + (y * g->w)];
}

int
caps(Grid *g, int x, int y)
{
	char c = get(g, x, y);
	return c >= 'A' && c <= 'Z';
}

void
parse(Grid *g)
{
	int i, x, y;
	for(i = 0; i < g->l; ++i) {
		char c = g->data[i];
		x = i % g->w;
		y = i / g->w;
		if(busy(g, x, y))
			continue;
		if(c == 'A') {
			set(g, x, y + 1, cchr(getint(g, x - 1, y) + getint(g, x + 1, y), caps(g, x + 1, y)));
			lock(g, x + 1, y);
			lock(g, x, y + 1);
		} else if(c == 'B') {
			set(g, x, y + 1, cchr(getint(g, x - 1, y) - getint(g, x + 1, y), caps(g, x + 1, y)));
			lock(g, x + 1, y);
			lock(g, x, y + 1);
		} else if(c == 'E') {
			if(x == g->w || get(g, x + 1, y) != '.')
				set(g, x, y, '*');
			else {
				set(g, x, y, '.');
				set(g, x + 1, y, 'E');
				lock(g, x + 1, y);
			}
		} else if(c == 'F') {
			/* TODO */
			lock(g, x + 1, y);
			lock(g, x, y + 1);
		} else if(c == 'L') {
			/* TODO */
			lock(g, x + 1, y);
			lock(g, x, y + 1);
		} else if(c == 'M') {
			/* TODO */
			lock(g, x + 1, y);
			lock(g, x, y + 1);
		} else if(c == 'N') {
			if(y == 0 || get(g, x, y - 1) != '.')
				set(g, x, y, '*');
			else {
				set(g, x, y, '.');
				set(g, x, y - 1, 'N');
				lock(g, x, y - 1);
			}
		} else if(c == 'S') {
			if(y == g->h || get(g, x, y + 1) != '.')
				set(g, x, y, '*');
			else {
				set(g, x, y, '.');
				set(g, x, y + 1, 'S');
				lock(g, x, y + 1);
			}
		} else if(c == 'W') {
			if(x == 0 || get(g, x - 1, y) != '.')
				set(g, x, y, '*');
			else {
				set(g, x, y, '.');
				set(g, x - 1, y, 'W');
				lock(g, x - 1, y);
			}
		}
	}
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
