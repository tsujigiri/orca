#include <stdio.h>

typedef struct Grid {
	int width;
	int height;
	int length;
	char data[256 * 256];
} Grid;

int
error(char *name)
{
	printf("Error: %s\n", name);
	return 0;
}

void
run(FILE *f, Grid *g)
{
	char c;
	g->length = 0;
	while((c = fgetc(f)) != EOF) {
		if(c == '\n') {
			if(g->width == 0)
				g->width = g->length;
			g->height = g->length / g->width;
		}
		g->length++;
	}
	printf("grid:%d(%dx%d)\n", g->length, g->width, g->height);
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
	run(f, &g);
	return 0;
}
