#include <stdio.h>

#define MAXSZ 128 * 128

typedef struct Grid {
	int w, h, l, f, r;
	int lock[MAXSZ];
	int type[MAXSZ];
	char vars[36];
	char data[MAXSZ];
} Grid;

/* Port types 
1 - locked
2 - port(left)
3 - operator
4 - port(right)
5 - port(output)
*/

int
ciuc(char c)
{
	return c >= 'A' && c <= 'Z';
}

int
cilc(char c)
{
	return c >= 'a' && c <= 'z';
}

int
cinu(char c)
{
	return c >= '0' && c <= '9';
}

int
cisp(char c)
{
	return !ciuc(c) && !cilc(c) && !cinu(c) && c != '.';
}

int
cint(char c)
{
	if(c == '.')
		return 0;
	if(cinu(c))
		return c - '0';
	if(cilc(c))
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
	return x >= 0 && x <= g->w && y >= 0 && y <= g->h;
}

int
random(Grid *g)
{
	g->r *= 1103515245;
	return ((g->r / 65536 * g->f) % 32768) ^ g->f;
}

/* IO */

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

/* Locks */

void
lock(Grid *g, int x, int y)
{
	if(valid(g, x, y)) {
		g->lock[x + (y * g->w)] = 1;
		g->type[x + (y * g->w)] = 1;
	}
}

/* Variables */

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

/* Syntax */

int
gettype(Grid *g, int x, int y)
{
	if(valid(g, x, y))
		return g->type[x + (y * g->w)];
	return 0;
}

void
settype(Grid *g, int x, int y, int t)
{
	if(valid(g, x, y))
		g->type[x + (y * g->w)] = t;
}

/* Port Setters */

void
setport(Grid *g, int x, int y, char c)
{
	lock(g, x, y);
	settype(g, x, y, 5);
	set(g, x, y, c);
}

int
getport(Grid *g, int x, int y, int l)
{
	if(l) {
		lock(g, x, y);
		settype(g, x, y, 4);
	} else
		settype(g, x, y, 2);
	return get(g, x, y);
}

int
bang(Grid *g, int x, int y)
{
	return get(g, x - 1, y) == '*' || get(g, x + 1, y) == '*' || get(g, x, y - 1) == '*' || get(g, x, y + 1) == '*';
}

/* Library */

void
op0(Grid *g, int x, int y, char c)
{
	(void)g;
	(void)x;
	(void)y;
	(void)c;
}

void
opa(Grid *g, int x, int y, char c)
{
	char a = getport(g, x - 1, y, 0);
	char b = getport(g, x + 1, y, 1);
	setport(g, x, y + 1, cchr(cint(a) + cint(b), ciuc(b)));
	(void)c;
}

void
opb(Grid *g, int x, int y, char c)
{
	char a = getport(g, x - 1, y, 0);
	char b = getport(g, x + 1, y, 1);
	setport(g, x, y + 1, cchr(cint(a) - cint(b), ciuc(b)));
	(void)c;
}

void
opc(Grid *g, int x, int y, char c)
{
	char rate = getport(g, x - 1, y, 0);
	char mod = getport(g, x + 1, y, 1);
	setport(g, x, y + 1, cchr(g->f / rate % mod, ciuc(mod)));
	(void)c;
}

void
opd(Grid *g, int x, int y, char c)
{
	char rate = getport(g, x - 1, y, 0);
	char mod = getport(g, x + 1, y, 1);
	setport(g, x, y + 1, g->f % (rate * mod) == 0 ? '*' : '.');
	(void)c;
}

void
ope(Grid *g, int x, int y, char c)
{
	if(x == g->w || get(g, x + 1, y) != '.')
		setport(g, x, y, '*');
	else {
		set(g, x, y, '.');
		setport(g, x + 1, y, c);
	}
}

void
opf(Grid *g, int x, int y, char c)
{
	setport(g, x, y + 1, getport(g, x - 1, y, 0) == getport(g, x + 1, y, 1) ? '*' : '.');
	(void)c;
}

void
opg(Grid *g, int x, int y, char c)
{
	int tx = cint(getport(g, x - 3, y, 0));
	int ty = cint(getport(g, x - 2, y, 0));
	int i, len = cint(getport(g, x - 1, y, 0));
	for(i = 0; i < len; ++i)
		setport(g, x + i + tx, y + 1 + ty, getport(g, x + 1 + i, y, 1));
	(void)c;
}

void
oph(Grid *g, int x, int y, char c)
{
	getport(g, x, y + 1, 1);
	(void)c;
}

void
opi(Grid *g, int x, int y, char c)
{
	char step = getport(g, x - 1, y, 0);
	char mod = getport(g, x + 1, y, 1);
	char val = getport(g, x, y + 1, 1);
	setport(g, x, y + 1, cchr((cint(val) + cint(step)) % (cint(mod) || 1), ciuc(mod)));
	(void)c;
}

void
opj(Grid *g, int x, int y, char c)
{
	int i;
	char link = getport(g, x, y - 1, 0);
	if(link != c) {
		for(i = 1; y + i < g->h; ++i)
			if(get(g, x, y + i) != c)
				break;
		setport(g, x, y + i, link);
	}
}

void
opk(Grid *g, int x, int y, char c)
{
	int i, len = cint(getport(g, x - 1, y, 0));
	for(i = 0; i < len; ++i) {
		char key = getport(g, x + 1 + i, y, 1);
		if(key == '.')
			continue;
		setport(g, x + 1 + i, y + 1, load(g, key));
	}
	(void)c;
}

void
opl(Grid *g, int x, int y, char c)
{
	char a = getport(g, x - 1, y, 0);
	char b = getport(g, x + 1, y, 1);
	setport(g, x, y + 1, cint(a) < cint(b) ? a : b);
	(void)c;
}

void
opm(Grid *g, int x, int y, char c)
{
	char a = getport(g, x - 1, y, 0);
	char b = getport(g, x + 1, y, 1);
	setport(g, x, y + 1, cchr(cint(a) * cint(b), ciuc(b)));
	(void)c;
}

void
opn(Grid *g, int x, int y, char c)
{
	if(y == 0 || get(g, x, y - 1) != '.')
		setport(g, x, y, '*');
	else {
		set(g, x, y, '.');
		setport(g, x, y - 1, c);
	}
}

void
opo(Grid *g, int x, int y, char c)
{
	int tx = cint(getport(g, x - 2, y, 0));
	int ty = cint(getport(g, x - 1, y, 0));
	setport(g, x, y + 1, getport(g, x + 1 + tx, y + ty, 1));
	(void)c;
}

void
opp(Grid *g, int x, int y, char c)
{
	int key = cint(getport(g, x - 2, y, 0));
	int i, len = cint(getport(g, x - 1, y, 0));
	/* TODO */
	for(i = 0; i < len; ++i)
		lock(g, x + i, y + 1);
	setport(g, x + (key % len), y + 1, get(g, x + 1, y));
	(void)c;
}

void
opq(Grid *g, int x, int y, char c)
{
	int tx = cint(getport(g, x - 3, y, 0));
	int ty = cint(getport(g, x - 2, y, 0));
	int i, len = cint(getport(g, x - 1, y, 0));
	for(i = 0; i < len; ++i)
		setport(g, x + 1 - len + i, y + 1, getport(g, x + 1 + tx + i, y + ty, 1));
	(void)c;
}

void
opr(Grid *g, int x, int y, char c)
{
	int min = cint(getport(g, x - 1, y, 0));
	char max = getport(g, x + 1, y, 1);
	setport(g, x, y + 1, cchr((random(g) % ((cint(max) - min) || 1)) + min, ciuc(max)));
	(void)c;
}

void
ops(Grid *g, int x, int y, char c)
{
	if(y == g->h || get(g, x, y + 1) != '.')
		setport(g, x, y, '*');
	else {
		set(g, x, y, '.');
		setport(g, x, y + 1, c);
	}
}

void
opt(Grid *g, int x, int y, char c)
{
	int key = cint(getport(g, x - 2, y, 0));
	int i, len = cint(getport(g, x - 1, y, 0));
	for(i = 0; i < len; ++i)
		lock(g, x + 1 + i, y);
	setport(g, x, y + 1, getport(g, x + 1 + (key % len), y, 1));
	(void)c;
}

void
opu(Grid *g, int x, int y, char c)
{
	int max = cint(getport(g, x - 1, y, 0));
	int step = cint(getport(g, x + 1, y, 1));
	int bucket = (step * (g->f + max - 1)) % max + step;
	setport(g, x, y + 1, bucket >= max ? '*' : '.');
	(void)c;
}

void
opv(Grid *g, int x, int y, char c)
{
	char w = getport(g, x - 1, y, 0);
	char r = getport(g, x + 1, y, 1);
	if(w != '.')
		save(g, w, r);
	else if(w == '.' && r != '.')
		setport(g, x, y + 1, load(g, r));
	(void)c;
}

void
opw(Grid *g, int x, int y, char c)
{
	if(x == 0 || get(g, x - 1, y) != '.')
		setport(g, x, y, '*');
	else {
		set(g, x, y, '.');
		setport(g, x - 1, y, c);
	}
}

void
opx(Grid *g, int x, int y, char c)
{
	int tx = cint(getport(g, x - 2, y, 0));
	int ty = cint(getport(g, x - 1, y, 0));
	setport(g, x + tx, y + ty + 1, getport(g, x + 1, y, 1));
	(void)c;
}

void
opy(Grid *g, int x, int y, char c)
{
	int i;
	char link = getport(g, x - 1, y, 0);
	if(link != c) {
		for(i = 1; x + i < g->w; ++i)
			if(get(g, x + i, y) != c)
				break;
		setport(g, x + i, y, link);
	}
}

void
opz(Grid *g, int x, int y, char c)
{
	int rate = cint(getport(g, x - 1, y, 0));
	char target = getport(g, x + 1, y, 1);
	char val = cint(getport(g, x, y + 1, 1));
	int t = cint(target);
	int mod = val < t ? rate : val > t ? -rate : 0;
	setport(g, x, y + 1, cchr(val + mod, ciuc(target)));
	(void)c;
}

/* clang-format off */

void (*library[36])(Grid *g, int x, int y, char c) = {
	op0, op0, op0, op0, op0, op0, 
	op0, op0, op0, op0, opa, opb, 
	opc, opd, ope, opf, opg, oph, 
	opi, opj, opk, opl, opm, opn, 
	opo, opp, opq, opr, ops, opt, 
	opu, opv, opw, opx, opy, opz
};

/* clang-format on */

void
opcomment(Grid *g, int x, int y)
{
	int i;
	for(i = 1; x + i < g->w; ++i) {
		lock(g, x + i, y);
		if(get(g, x + i, y) == '#')
			break;
	}
}

void
opspecial(Grid *g, int x, int y)
{
	int i, b = bang(g, x, y);
	for(i = 0; x + i < g->w; ++i) {
		if(get(g, x + i, y) == '.')
			break;
		lock(g, x + i, y);
		if(b)
			printf("%c", get(g, x + i, y));
	}
	if(b)
		printf("\n");
}

void
opdefault(Grid *g, int x, int y, char c)
{
	settype(g, x, y, 3);
	library[cint(c)](g, x, y, c);
}

/* General */

void
print(Grid *g)
{
	int x, y;
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
}

int
run(Grid *g)
{
	int i, x, y;
	for(i = 0; i < g->l; ++i) {
		g->lock[i] = 0;
		g->type[i] = 0;
	}
	for(i = 0; i < g->l; ++i) {
		char c = g->data[i];
		x = i % g->w;
		y = i / g->w;
		if(g->lock[i])
			continue;
		if(c == '*')
			set(g, x, y, '.');
		else if(c == '#')
			opcomment(g, x, y);
		else if(cisp(c))
			opspecial(g, x, y);
		else if(ciuc(c) || (cilc(c) && bang(g, x, y)))
			opdefault(g, x, y, c);
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
error(char *name)
{
	printf("Error: %s\n", name);
	return 0;
}

int
main(int argc, char *argv[])
{
	FILE *f;
	int limit = 100;
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
