#include <stdio.h>

#define MAXSZ 256 * 256

typedef struct Grid {
	int w, h, l, f, r;
	int lock[MAXSZ];
	char vars[36];
	char data[MAXSZ];
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

int
valid(Grid *g, int x, int y)
{
	return x >= 0 || x < g->w || y >= 0 || y < g->h;
}

char
get(Grid *g, int x, int y)
{
	if(valid(g, x, y))
		return g->data[x + (y * g->w)];
	return '.';
}

void
set(Grid *g, int x, int y, char c)
{
	if(valid(g, x, y))
		g->data[x + (y * g->w)] = c;
}

void
lock(Grid *g, int x, int y)
{
	if(valid(g, x, y))
		g->lock[x + (y * g->w)] = 1;
}

void
setlock(Grid *g, int x, int y, char c)
{
	set(g, x, y, c);
	lock(g, x, y);
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

int
bang(Grid *g, int x, int y)
{
	return get(g, x - 1, y) == '*' || get(g, x + 1, y) == '*' || get(g, x, y - 1) == '*' || get(g, x, y + 1) == '*';
}

int
random(Grid *g)
{
	g->r *= 1103515245;
	return ((g->r / 65536 * g->f) % 32768) ^ g->f;
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
	lock(g, x + 1, y);
	setlock(g, x, y + 1, cchr(cint(a) + cint(b), ciuc(b)));
}

void
opb(Grid *g, int x, int y)
{
	char a = get(g, x - 1, y);
	char b = get(g, x + 1, y);
	lock(g, x + 1, y);
	setlock(g, x, y + 1, cchr(cint(a) - cint(b), ciuc(b)));
}

void
opc(Grid *g, int x, int y)
{
	char rate = get(g, x - 1, y);
	char mod = get(g, x + 1, y);
	lock(g, x, y + 1);
	setlock(g, x, y + 1, cchr(g->f / rate % mod, ciuc(mod)));
}

void
opd(Grid *g, int x, int y)
{
	char rate = get(g, x - 1, y);
	char mod = get(g, x + 1, y);
	lock(g, x + 1, y);
	setlock(g, x, y + 1, g->f % (rate * mod) == 0 ? '*' : '.');
}

void
ope(Grid *g, int x, int y)
{
	char c = get(g, x, y);
	if(x == g->w || get(g, x + 1, y) != '.')
		setlock(g, x, y, '*');
	else {
		set(g, x, y, '.');
		setlock(g, x + 1, y, c);
	}
}

void
opf(Grid *g, int x, int y)
{
	lock(g, x + 1, y);
	setlock(g, x, y + 1, get(g, x - 1, y) == get(g, x + 1, y) ? '*' : '.');
}

void
opg(Grid *g, int x, int y)
{
	int tx = cint(get(g, x - 3, y));
	int ty = cint(get(g, x - 2, y));
	int i, len = cint(get(g, x - 1, y));
	for(i = 0; i < len; ++i)
		setlock(g, x + i + tx, y + 1 + ty, get(g, x + 1 + i, y));
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
	setlock(g, x, y + 1, cchr((cint(val) + cint(step)) % cint(mod), ciuc(mod)));
}

void
opj(Grid *g, int x, int y)
{
	setlock(g, x, y + 1, get(g, x, y - 1));
}

void
opk(Grid *g, int x, int y)
{
	int i, len = cint(get(g, x - 1, y));
	for(i = 0; i < len; ++i) {
		char key = get(g, x + 1 + i, y);
		if(key == '.')
			continue;
		setlock(g, x + 1 + i, y + 1, load(g, key));
	}
}

void
opl(Grid *g, int x, int y)
{
	char a = get(g, x - 1, y);
	char b = get(g, x + 1, y);
	lock(g, x + 1, y);
	setlock(g, x, y + 1, cint(a) < cint(b) ? a : b);
}

void
opm(Grid *g, int x, int y)
{
	char a = get(g, x - 1, y);
	char b = get(g, x + 1, y);
	lock(g, x + 1, y);
	setlock(g, x, y + 1, cchr(cint(a) * cint(b), ciuc(b)));
}

void
opn(Grid *g, int x, int y)
{
	char c = get(g, x, y);
	if(y == 0 || get(g, x, y - 1) != '.')
		setlock(g, x, y, '*');
	else {
		set(g, x, y, '.');
		setlock(g, x, y - 1, c);
	}
}

void
opo(Grid *g, int x, int y)
{
	int tx = cint(get(g, x - 2, y));
	int ty = cint(get(g, x - 1, y));
	lock(g, x + 1 + tx, y + ty);
	set(g, x, y + 1, get(g, x + 1 + tx, y + ty));
}

void
opp(Grid *g, int x, int y)
{
	int key = cint(get(g, x - 2, y));
	int i, len = cint(get(g, x - 1, y));
	for(i = 0; i < len; ++i)
		lock(g, x + i, y + 1);
	set(g, x + (key % len), y + 1, get(g, x + 1, y));
}

void
opq(Grid *g, int x, int y)
{
	int tx = cint(get(g, x - 3, y));
	int ty = cint(get(g, x - 2, y));
	int i, len = cint(get(g, x - 1, y));
	for(i = 0; i < len; ++i)
		setlock(g, x + 1 - len + i, y + 1, get(g, x + 1 + tx + i, y + ty));
}

void
opr(Grid *g, int x, int y)
{
	int min = cint(get(g, x - 1, y));
	char max = get(g, x + 1, y);
	lock(g, x + 1, y);
	setlock(g, x, y + 1, cchr((random(g) % (cint(max) - min)) + min, ciuc(max)));
}

void
ops(Grid *g, int x, int y)
{
	char c = get(g, x, y);
	if(y == g->h || get(g, x, y + 1) != '.')
		setlock(g, x, y, '*');
	else {
		set(g, x, y, '.');
		setlock(g, x, y + 1, c);
	}
}

void
opt(Grid *g, int x, int y)
{
	int key = cint(get(g, x - 2, y));
	int i, len = cint(get(g, x - 1, y));
	for(i = 0; i < len; ++i)
		lock(g, x + 1 + i, y);
	setlock(g, x, y + 1, get(g, x + 1 + (key % len), y));
}

void
opu(Grid *g, int x, int y)
{
	int max = cint(get(g, x - 1, y));
	int step = cint(get(g, x + 1, y));
	int bucket = (step * (g->f + max - 1)) % max + step;
	lock(g, x + 1, y);
	setlock(g, x, y + 1, bucket >= max ? '*' : '.');
}

void
opv(Grid *g, int x, int y)
{
	char w = get(g, x - 1, y);
	char r = get(g, x + 1, y);
	lock(g, x + 1, y);
	if(w != '.')
		save(g, w, r);
	else if(w == '.' && r != '.')
		setlock(g, x, y + 1, load(g, r));
}

void
opw(Grid *g, int x, int y)
{
	char c = get(g, x, y);
	if(x == 0 || get(g, x - 1, y) != '.')
		setlock(g, x, y, '*');
	else {
		set(g, x, y, '.');
		setlock(g, x - 1, y, c);
	}
}

void
opx(Grid *g, int x, int y)
{
	int tx = cint(get(g, x - 2, y));
	int ty = cint(get(g, x - 1, y));
	lock(g, x + 1, y);
	setlock(g, x + tx, y + ty + 1, get(g, x + 1, y));
}

void
opy(Grid *g, int x, int y)
{
	setlock(g, x + 1, y, get(g, x - 1, y));
}

void
opz(Grid *g, int x, int y)
{
	int rate = cint(get(g, x - 1, y));
	char target = get(g, x + 1, y);
	char val = cint(get(g, x, y + 1));
	int t = cint(target);
	int mod = val < t ? rate : val > t ? -rate : 0;
	lock(g, x + 1, y);
	setlock(g, x, y + 1, cchr(val + mod, ciuc(target)));
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
print(Grid *g)
{
	int i;
	for(i = 0; i < g->l; ++i)
		putchar(g->data[i]);
	putchar('\n');
}

int
run(Grid *g)
{
	int i, x, y, comment = 0;
	for(i = 0; i < g->l; ++i)
		g->lock[i] = 0;
	for(i = 0; i < g->l; ++i) {
		char c = g->data[i];
		x = i % g->w;
		y = i / g->w;
		if(x == 0)
			comment = 0;
		if(c == '#')
			comment = !comment;
		if(comment || g->lock[i])
			continue;
		if(c == '*')
			set(g, x, y, '.');
		if(!ciuc(c) && !bang(g, x, y))
			continue;
		library[cint(c)](g, x, y);
	}
	g->f++;
	return 1;
}

int
disk(FILE *f, Grid *g)
{
	char c;
	g->l = 0;
	while((c = fgetc(f)) != EOF && g->l < MAXSZ) {
		if(c == '\n') {
			if(g->w == 0)
				g->w = g->l + 1;
			g->h = g->l / g->w;
		}
		g->data[g->l++] = c;
	}
	return g->w > 2 && g->h > 2;
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
