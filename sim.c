#include <stdio.h>
#include "sim.h"

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
clca(int c)
{
	return ciuc(c) ? c + ('a' - 'A') : c;
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
	return x >= 0 && x <= g->w - 1 && y >= 0 && y <= g->h - 1;
}

int
random(Grid *g)
{
	(void)g;
	return 0;
	/*
	g->r *= 1103515245;
	return ((g->r / 65536 * g->f) % 32768) ^ g->f;
	*/
}

int
base36(char c)
{
	if(c >= 'A' && c <= 'Z')
		return c - 'A' + 10;
	if(c >= 'a' && c <= 'z')
		return c - 'a' + 10;
	if(c >= '0' && c <= '9')
		return c - '0';
	return 0;
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

/* Variables */

void
save(Grid *g, char key, char val)
{
	g->vars[base36(key)] = val;
}

char
load(Grid *g, char key)
{
	return g->vars[base36(key)];
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

/* Locks */

void
lock(Grid *g, int x, int y)
{
	if(valid(g, x, y)) {
		g->lock[x + (y * g->w)] = 1;
		if(!gettype(g, x, y))
			settype(g, x, y, 1);
	}
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
opa(Grid *g, int x, int y, char c)
{
	char a = getport(g, x - 1, y, 0);
	char b = getport(g, x + 1, y, 1);
	setport(g, x, y + 1, cchr(base36(a) + base36(b), ciuc(b)));
	(void)c;
}

void
opb(Grid *g, int x, int y, char c)
{
	char a = getport(g, x - 1, y, 0);
	char b = getport(g, x + 1, y, 1);
	setport(g, x, y + 1, cchr(base36(a) - base36(b), ciuc(b)));
	(void)c;
}

void
opc(Grid *g, int x, int y, char c)
{
	char rate = getport(g, x - 1, y, 0);
	char mod = getport(g, x + 1, y, 1);
	int mod_ = base36(mod);
	int rate_ = base36(rate);
	if(!rate_)
		rate_ = 1;
	if(!mod_)
		mod_ = 8;
	setport(g, x, y + 1, cchr(g->f / rate_ % mod_, ciuc(mod)));
	(void)c;
}

void
opd(Grid *g, int x, int y, char c)
{
	char rate = getport(g, x - 1, y, 0);
	char mod = getport(g, x + 1, y, 1);
	int rate_ = base36(rate);
	int mod_ = base36(mod);
	if(!rate_)
		rate_ = 1;
	if(!mod_)
		mod_ = 8;
	setport(g, x, y + 1, g->f % (rate_ * mod_) == 0 ? '*' : '.');
	(void)c;
}

void
ope(Grid *g, int x, int y, char c)
{
	if(!valid(g, x + 1, y) || get(g, x + 1, y) != '.')
		set(g, x, y, '*');
	else {
		set(g, x, y, '.');
		setport(g, x + 1, y, c);
		settype(g, x + 1, y, 0);
	}
	settype(g, x, y, 0);
}

void
opf(Grid *g, int x, int y, char c)
{
	char a = getport(g, x - 1, y, 0);
	char b = getport(g, x + 1, y, 1);
	setport(g, x, y + 1, a == b ? '*' : '.');
	(void)c;
}

void
opg(Grid *g, int x, int y, char c)
{
	char px = getport(g, x - 3, y, 0);
	char py = getport(g, x - 2, y, 0);
	char len = getport(g, x - 1, y, 0);
	int i, len_ = base36(len);
	if(!len_)
		len_ = 1;
	for(i = 0; i < len_; ++i)
		setport(g, x + i + base36(px), y + 1 + base36(py), getport(g, x + 1 + i, y, 1));
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
	char rate = getport(g, x - 1, y, 0);
	char mod = getport(g, x + 1, y, 1);
	char val = getport(g, x, y + 1, 1);
	int rate_ = base36(rate);
	int mod_ = base36(mod);
	if(!rate_)
		rate_ = 1;
	if(!mod_)
		mod_ = 36;
	setport(g, x, y + 1, cchr((base36(val) + rate_) % mod_, ciuc(mod)));
	(void)c;
}

void
opj(Grid *g, int x, int y, char c)
{
	char link = getport(g, x, y - 1, 0);
	int i;
	if(link != c) {
		for(i = 1; y + i < 256; ++i)
			if(get(g, x, y + i) != c)
				break;
		setport(g, x, y + i, link);
	}
}

void
opk(Grid *g, int x, int y, char c)
{
	char len = getport(g, x - 1, y, 0);
	int i, len_ = base36(len);
	if(!len_)
		len_ = 1;
	for(i = 0; i < len_; ++i) {
		char key = getport(g, x + 1 + i, y, 1);
		if(key != '.')
			setport(g, x + 1 + i, y + 1, load(g, key));
	}
	(void)c;
}

void
opl(Grid *g, int x, int y, char c)
{
	char a = getport(g, x - 1, y, 0);
	char b = getport(g, x + 1, y, 1);
	setport(g, x, y + 1, base36(a) < base36(b) ? a : b);
	(void)c;
}

void
opm(Grid *g, int x, int y, char c)
{
	char a = getport(g, x - 1, y, 0);
	char b = getport(g, x + 1, y, 1);
	setport(g, x, y + 1, cchr(base36(a) * base36(b), ciuc(b)));
	(void)c;
}

void
opn(Grid *g, int x, int y, char c)
{
	if(!valid(g, x, y - 1) || get(g, x, y - 1) != '.')
		set(g, x, y, '*');
	else {
		set(g, x, y, '.');
		setport(g, x, y - 1, c);
		settype(g, x, y - 1, 0);
	}
	settype(g, x, y, 0);
}

void
opo(Grid *g, int x, int y, char c)
{
	char px = getport(g, x - 2, y, 0);
	char py = getport(g, x - 1, y, 0);
	setport(g, x, y + 1, getport(g, x + 1 + base36(px), y + base36(py), 1));
	(void)c;
}

void
opp(Grid *g, int x, int y, char c)
{
	char key = getport(g, x - 2, y, 0);
	char len = getport(g, x - 1, y, 0);
	char val = getport(g, x + 1, y, 1);
	int i, len_ = base36(len);
	if(!len_)
		len_ = 1;
	for(i = 0; i < len_; ++i)
		lock(g, x + i, y + 1);
	setport(g, x + (base36(key) % len_), y + 1, val);
	(void)c;
}

void
opq(Grid *g, int x, int y, char c)
{
	char px = getport(g, x - 3, y, 0);
	char py = getport(g, x - 2, y, 0);
	char len = getport(g, x - 1, y, 0);
	int i, len_ = base36(len);
	if(!len_)
		len_ = 1;
	for(i = 0; i < len_; ++i)
		setport(g, x + 1 - len_ + i, y + 1, getport(g, x + 1 + base36(px) + i, y + base36(py), 1));
	(void)c;
}

void
opr(Grid *g, int x, int y, char c)
{
	char min = getport(g, x - 1, y, 0);
	char max = getport(g, x + 1, y, 1);
	int min_ = base36(min);
	int max_ = base36(max);
	setport(g, x, y + 1, cchr((random(g) % ((base36(max_) - min_) || 1)) + min_, ciuc(max)));
	(void)c;
}

void
ops(Grid *g, int x, int y, char c)
{
	if(!valid(g, x, y + 1) || get(g, x, y + 1) != '.')
		set(g, x, y, '*');
	else {
		set(g, x, y, '.');
		setport(g, x, y + 1, c);
		settype(g, x, y + 1, 0);
	}
	settype(g, x, y, 0);
}

void
opt(Grid *g, int x, int y, char c)
{
	char key = getport(g, x - 2, y, 0);
	char len = getport(g, x - 1, y, 0);
	int i, len_ = base36(len);
	if(!len_)
		len_ = 1;
	for(i = 0; i < len_; ++i)
		lock(g, x + 1 + i, y);
	setport(g, x, y + 1, getport(g, x + 1 + (base36(key) % len_), y, 1));
	(void)c;
}

void
opu(Grid *g, int x, int y, char c)
{
	char step = getport(g, x - 1, y, 1);
	char max = getport(g, x + 1, y, 1);
	int step_ = base36(step);
	int max_ = base36(max);
	int bucket;
	if(!step_)
		step_ = 1;
	if(!max_)
		max_ = 8;
	bucket = (step_ * (g->f + max_ - 1)) % max_ + step_;
	setport(g, x, y + 1, bucket >= max_ ? '*' : '.');
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
	if(!valid(g, x - 1, y) || get(g, x - 1, y) != '.')
		set(g, x, y, '*');
	else {
		set(g, x, y, '.');
		setport(g, x - 1, y, c);
		settype(g, x - 1, y, 0);
	}
	settype(g, x, y, 0);
}

void
opx(Grid *g, int x, int y, char c)
{
	char px = getport(g, x - 2, y, 0);
	char py = getport(g, x - 1, y, 0);
	char val = getport(g, x + 1, y, 1);
	setport(g, x + base36(px), y + base36(py) + 1, val);
	(void)c;
}

void
opy(Grid *g, int x, int y, char c)
{
	char link = getport(g, x - 1, y, 0);
	int i;
	if(link != c) {
		for(i = 1; x + i < 256; ++i)
			if(get(g, x + i, y) != c)
				break;
		setport(g, x + i, y, link);
	}
}

void
opz(Grid *g, int x, int y, char c)
{
	char rate = getport(g, x - 1, y, 0);
	char target = getport(g, x + 1, y, 1);
	char val = getport(g, x, y + 1, 1);
	int rate_ = base36(rate);
	int target_ = base36(target);
	int val_ = base36(val);
	int mod;
	if(!rate_)
		rate_ = 1;
	if(val_ <= target_ - rate_)
		mod = rate_;
	else if(val_ >= target_ + rate_)
		mod = -rate;
	else
		mod = target_ - val_;
	setport(g, x, y + 1, cchr(val_ + mod, ciuc(target)));
	(void)c;
}

void
opcomment(Grid *g, int x, int y)
{
	int i;
	for(i = 1; x + i < 256; ++i) {
		lock(g, x + i, y);
		if(get(g, x + i, y) == '#')
			break;
	}
	settype(g, x, y, 1);
}

void
opspecial(Grid *g, int x, int y)
{
	int i, b = bang(g, x, y);
	for(i = 0; x + i < MSGSZ; ++i) {
		char c = getport(g, x + i, y, 1);
		if(c == '.')
			break;
		if(b && g->msglen < MSGSZ)
			g->msg[g->msglen++] = c;
	}
	settype(g, x, y, b ? 3 : 2);
}

void
operate(Grid *g, int x, int y, char c)
{
	settype(g, x, y, 3);
	switch(clca(c)) {
	case 'a': opa(g, x, y, c); break;
	case 'b': opb(g, x, y, c); break;
	case 'c': opc(g, x, y, c); break;
	case 'd': opd(g, x, y, c); break;
	case 'e': ope(g, x, y, c); break;
	case 'f': opf(g, x, y, c); break;
	case 'g': opg(g, x, y, c); break;
	case 'h': oph(g, x, y, c); break;
	case 'i': opi(g, x, y, c); break;
	case 'k': opk(g, x, y, c); break;
	case 'j': opj(g, x, y, c); break;
	case 'l': opl(g, x, y, c); break;
	case 'm': opm(g, x, y, c); break;
	case 'n': opn(g, x, y, c); break;
	case 'o': opo(g, x, y, c); break;
	case 'p': opp(g, x, y, c); break;
	case 'q': opq(g, x, y, c); break;
	case 'r': opr(g, x, y, c); break;
	case 's': ops(g, x, y, c); break;
	case 't': opt(g, x, y, c); break;
	case 'u': opu(g, x, y, c); break;
	case 'v': opv(g, x, y, c); break;
	case 'w': opw(g, x, y, c); break;
	case 'x': opx(g, x, y, c); break;
	case 'y': opy(g, x, y, c); break;
	case 'z': opz(g, x, y, c); break;
	case '*': set(g, x, y, '.'); break;
	case '#': opcomment(g, x, y); break;
	default: opspecial(g, x, y);
	}
}

/* General */

void
initframe(Grid *g)
{
	int i;
	for(i = 0; i < g->l; ++i) {
		g->lock[i] = 0;
		g->type[i] = 0;
	}
	g->msg[0] = '\0';
	g->msglen = 0;
}

int
rungrid(Grid *g)
{
	int i, x, y;
	initframe(g);
	for(i = 0; i < g->l; ++i) {
		char c = g->data[i];
		x = i % g->w;
		y = i / g->w;
		if(c == '.' || g->lock[i])
			continue;
		if(cinu(c))
			continue;
		if(cilc(c) && !bang(g, x, y))
			continue;
		operate(g, x, y, c);
	}
	g->f++;
	return 1;
}

int
loadgrid(Grid *g, FILE *f)
{
	int x = 0, y = 0;
	char c;
	if(!f)
		return 0;
	while((c = fgetc(f)) != EOF && g->l < MAXSZ) {
		if(c == '\n') {
			x = 0;
			y++;
		} else {
			set(g, x, y, c);
			x++;
		}
	}
	return 1;
}

void
initgrid(Grid *g, int w, int h)
{
	int i;
	g->w = w;
	g->h = h;
	g->l = w * h;
	g->f = 0;
	g->r = 1;
	for(i = 0; i < g->l; ++i)
		g->data[i] = '.';
	initframe(g);
}
