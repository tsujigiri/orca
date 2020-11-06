#include <stdio.h>

typedef struct Grid {
	int w, h, l, f;
	int lock[256 * 256];
	char vars[36];
	char data[256 * 256];
} Grid;

int
ciuc(char c)
{
	return c >= 'A' && c <= 'Z';
}

int
cint(char c)
{
	if(c == '.')
		return 0;
	if(c >= '0' && c <= '9')
		return c - '0';
	if(c >= 'a' && c <= 'z')
		return c - 'a' + 10;
	if(ciuc(c))
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

void
lock(Grid *g, int x, int y)
{
	g->lock[x + (y * g->w)] = 1;
}

void
save(Grid *g, char key, char val)
{
	g->vars[cint(key)] = val;
}

char
load(Grid *g, char key)
{
	return g->vars[cint(key)];
}

/* Library */

void
op0(void)
{
}

void
opa(Grid *g, int x, int y)
{
	char a = get(g, x - 1, y);
	char b = get(g, x + 1, y);
	set(g, x, y + 1, cchr(cint(a) + cint(b), ciuc(b)));
	lock(g, x + 1, y);
	lock(g, x, y + 1);
}

void
opb(Grid *g, int x, int y)
{
	char a = get(g, x - 1, y);
	char b = get(g, x + 1, y);
	set(g, x, y + 1, cchr(cint(a) - cint(b), ciuc(b)));
	lock(g, x + 1, y);
	lock(g, x, y + 1);
}

void
opc(Grid *g, int x, int y)
{
	char rate = get(g, x - 1, y);
	char mod = get(g, x + 1, y);
	set(g, x, y + 1, cchr(g->f / rate % mod, ciuc(mod)));
	lock(g, x, y + 1);
	lock(g, x, y + 1);
}

void
opd(Grid *g, int x, int y)
{
	char rate = get(g, x - 1, y);
	char mod = get(g, x + 1, y);
	set(g, x, y + 1, g->f % (rate * mod) == 0 ? '*' : '.');
	lock(g, x, y + 1);
	lock(g, x, y + 1);
}

void
ope(Grid *g, int x, int y)
{
	if(x == g->w || get(g, x + 1, y) != '.')
		set(g, x, y, '*');
	else {
		set(g, x, y, '.');
		set(g, x + 1, y, 'E');
		lock(g, x + 1, y);
	}
}

void
opf(Grid *g, int x, int y)
{
	set(g, x, y + 1, get(g, x - 1, y) == get(g, x + 1, y) ? '*' : '.');
	lock(g, x + 1, y);
	lock(g, x, y + 1);
}

void
opg(Grid *g, int x, int y)
{
	int tx = cint(get(g, x - 3, y));
	int ty = cint(get(g, x - 2, y));
	int len = cint(get(g, x - 1, y));
	int i;
	for(i = 0; i < len; ++i) {
		set(g, x + i + tx, y + 1 + ty, get(g, x + 1 + i, y));
		lock(g, x + i + tx, y + 1 + ty);
	}
}

void
oph(Grid *g, int x, int y)
{
	lock(g, x, y + 1);
}

void
opi(Grid *g, int x, int y)
{
	char step = get(g, x - 1, y);
	char mod = get(g, x + 1, y);
	char val = get(g, x, y + 1);
	set(g, x, y + 1, cchr((cint(val) + cint(step)) % cint(mod), ciuc(mod)));
	lock(g, x, y + 1);
}

void
opj(Grid *g, int x, int y)
{
	set(g, x, y + 1, get(g, x, y - 1));
	lock(g, x, y + 1);
}

void
opk(Grid *g, int x, int y)
{
	int len = cint(get(g, x - 1, y));
	int i;
	for(i = 0; i < len; ++i) {
		char key = get(g, x + 1 + i, y);
		if(key == '.')
			continue;
		set(g, x + 1 + i, y + 1, load(g, key));
		lock(g, x + 1 + i, y + 1);
	}
}

void
opl(Grid *g, int x, int y)
{
	char a = get(g, x - 1, y);
	char b = get(g, x + 1, y);
	set(g, x, y + 1, cint(a) < cint(b) ? a : b);
	lock(g, x + 1, y);
	lock(g, x, y + 1);
}

void
opm(Grid *g, int x, int y)
{
	char a = get(g, x - 1, y);
	char b = get(g, x + 1, y);
	set(g, x, y + 1, cchr(cint(a) * cint(b), ciuc(b)));
	lock(g, x + 1, y);
	lock(g, x, y + 1);
}

void
opn(Grid *g, int x, int y)
{
	if(y == 0 || get(g, x, y - 1) != '.')
		set(g, x, y, '*');
	else {
		set(g, x, y, '.');
		set(g, x, y - 1, 'N');
		lock(g, x, y - 1);
	}
}

void
opo(Grid *g, int x, int y)
{
	int tx = cint(get(g, x - 2, y));
	int ty = cint(get(g, x - 1, y));
	set(g, x, y + 1, get(g, x + 1 + tx, y + ty));
	lock(g, x + 1 + tx, y + ty);
}

void
opp(Grid *g, int x, int y)
{
	int key = cint(get(g, x - 2, y));
	int len = cint(get(g, x - 1, y));
	int i;
	for(i = 0; i < len; ++i)
		lock(g, x + i, y + 1);
	set(g, x + (key % len), y + 1, get(g, x + 1, y));
}

void
opq(Grid *g, int x, int y)
{
}

void
opr(Grid *g, int x, int y)
{
}

void
ops(Grid *g, int x, int y)
{
	if(y == g->h || get(g, x, y + 1) != '.')
		set(g, x, y, '*');
	else {
		set(g, x, y, '.');
		set(g, x, y + 1, 'S');
		lock(g, x, y + 1);
	}
}

void
opt(Grid *g, int x, int y)
{
	int key = cint(get(g, x - 2, y));
	int len = cint(get(g, x - 1, y));
	int i;
	for(i = 0; i < len; ++i)
		lock(g, x + 1 + i, y);
	set(g, x, y + 1, get(g, x + 1 + (key % len), y));
	lock(g, x, y + 1);
}

void
opu(Grid *g, int x, int y)
{
	int max = cint(get(g, x - 1, y));
	int step = cint(get(g, x + 1, y));
	int bucket = (step * (g->f + max - 1)) % max + step;
	set(g, x, y + 1, bucket >= max ? '*' : '.');
	lock(g, x + 1, y);
	lock(g, x, y + 1);
}

void
opv(Grid *g, int x, int y)
{
	char w = get(g, x - 1, y);
	char r = get(g, x + 1, y);
	if(w != '.')
		save(g, w, r);
	else if(w == '.' && r != '.') {
		set(g, x, y + 1, load(g, r));
		lock(g, x, y + 1);
	}
	lock(g, x + 1, y);
}

void
opw(Grid *g, int x, int y)
{
	if(x == 0 || get(g, x - 1, y) != '.')
		set(g, x, y, '*');
	else {
		set(g, x, y, '.');
		set(g, x - 1, y, 'W');
		lock(g, x - 1, y);
	}
}

void
opx(Grid *g, int x, int y)
{
	int tx = cint(get(g, x - 2, y));
	int ty = cint(get(g, x - 1, y));
	set(g, x + tx, y + ty + 1, get(g, x + 1, y));
	lock(g, x + 1, y);
	lock(g, x + tx, y + ty + 1);
}

void
opy(Grid *g, int x, int y)
{
	set(g, x + 1, y, get(g, x - 1, y));
	lock(g, x + 1, y);
}

void
opz(Grid *g, int x, int y)
{
	int rate = cint(get(g, x - 1, y));
	char target = get(g, x + 1, y);
	char val = cint(get(g, x, y + 1));
	int t = cint(target);
	int mod = val < t ? rate : val > t ? -rate : 0;
	set(g, x, y + 1, cchr(val + mod, ciuc(target)));
	lock(g, x + 1, y);
	lock(g, x, y + 1);
}

/* clang-format off */

void (*library[36])() = {
	op0, op0, op0, op0, op0, op0, 
	op0, op0, op0, op0, opa, opb, 
	opc, opd, ope, opf, opg, oph, 
	opi, opj, opk, opl, opm, opn, 
	opo, opp, opq, opr, ops, opt, 
	opu, opv, opw, opx, opy, opz
};

/* clang-format on */

void
parse(Grid *g)
{
	int i, x, y;
	for(i = 0; i < g->l; ++i) {
		char c = g->data[i];
		x = i % g->w;
		y = i / g->w;
		if(g->lock[i])
			continue;
		if(!ciuc(c))
			continue;
		library[cint(c)](g, x, y);
	}
	for(i = 0; i < g->l; ++i)
		printf("%c", g->data[i]);
}

void
disk(FILE *f, Grid *g)
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
error(char *name)
{
	printf("Error: %s\n", name);
	return 0;
}

int
main(int argc, char *argv[])
{
	FILE *f;
	Grid g;
	g.f = 0;
	if(argc < 2)
		return error("No input.");
	f = fopen(argv[1], "r");
	if(f == NULL)
		return error("Missing input.");
	disk(f, &g);
	return 0;
}
