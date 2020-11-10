#include <stdio.h>
#include "sim.h"

int
error(char *name)
{
	printf("Error: %s\n", name);
	return 0;
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
	if(!disk(f, &g))
		return error("Invalid grid");
	while(g.f < limit) {
		run(&g);
		print(&g);
	}
	return 0;
}
