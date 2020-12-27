#include <stdio.h>
#include "sim.h"

int
error(char *name)
{
	printf("Error: %s\n", name);
	return 0;
}

void
printgrid(Grid *g)
{
	/* TODO: only print once, merge into a single buf */
	int x, y, i = 0;
	for(y = 0; y < g->h; ++y)
		for(x = 0; x < g->w; ++x) {
			putchar(get(g, x, y));
			if(x == g->w - 1)
				putchar('\n');
		}
	putchar('\n');
	for(y = 0; y < g->h; ++y)
		for(x = 0; x < g->w; ++x) {
			printf("%d", gettype(g, x, y));
			if(x == g->w - 1)
				putchar('\n');
		}
	putchar('\n');
	while(g->msg[i])
		putchar(g->msg[i++]);
	putchar('\n');
}

int
opengrid(Grid *g, FILE *f)
{
	char c;
	g->l = 0;
	while((c = fgetc(f)) != EOF && g->l < MAXSZ) {
		if(c == '\n') {
			if(g->w == 0)
				g->w = g->l;
			g->h = g->l / g->w;
		} else {
			g->type[g->l] = 0;
			g->data[g->l++] = c;
		}
	}
	return g->w > 2 && g->h > 2;
}

int
main(int argc, char *argv[])
{
	FILE *f;
	int limit = 3;
	Grid g;
	g.w = 0;
	g.h = 0;
	g.f = 0;
	g.r = 1;
	if(argc < 2)
		return error("No input.");
	f = fopen(argv[1], "r");
	if(!f)
		return error("Missing input.");
	if(!opengrid(&g, f))
		return error("Invalid grid");
	while(g.f < limit) {
		rungrid(&g);
		printgrid(&g);
	}
	return 0;
}
