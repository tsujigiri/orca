#include <stdio.h>
#include <SDL2/SDL.h>
#include <portmidi.h>
#include <porttime.h>
#include "sim.h"

/* 
Copyright (c) 2020 Devine Lu Linvega

Permission to use, copy, modify, and distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE.
*/

#define HOR 32
#define VER 16
#define PAD 8
#define VOICES 16
#define DEVICE 0

#define SZ (HOR * VER * 16)
#define CLIPSZ 1024

typedef unsigned char Uint8;

typedef struct {
	char name[256];
} Document;

typedef struct {
	int x, y, w, h;
} Rect2d;

typedef struct {
	int channel, value, velocity, length;
} Note;

Document doc;
char clip[CLIPSZ];
Note voices[VOICES];
Rect2d cursor;
Grid g;

int WIDTH = 8 * HOR + PAD * 2;
int HEIGHT = 8 * (VER + 2) + PAD * 2;
int BPM = 128, DOWN = 0, ZOOM = 2, PAUSE = 0, GUIDES = 1, MODE = 0;

Uint32 theme[] = {
	0x000000,
	0xFFFFFF,
	0x72DEC2,
	0x666666,
	0xffb545};

Uint8 icons[][8] = {
	{0x00, 0x00, 0x10, 0x38, 0x7c, 0x38, 0x10, 0x00}, /* play */
	{0x00, 0x00, 0x48, 0x24, 0x12, 0x24, 0x48, 0x00}, /* next */
	{0x00, 0x00, 0x66, 0x42, 0x00, 0x42, 0x66, 0x00}, /* skip */
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3e, 0x00}, /* midi:1 */
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x3e, 0x3e, 0x00}, /* midi:2 */
	{0x00, 0x00, 0x00, 0x00, 0x3e, 0x3e, 0x3e, 0x00}, /* midi:3 */
	{0x00, 0x00, 0x00, 0x3e, 0x3e, 0x3e, 0x3e, 0x00}, /* midi:4 */
	{0x00, 0x00, 0x3e, 0x3e, 0x3e, 0x3e, 0x3e, 0x00}, /* midi:5 */
	{0x00, 0x3e, 0x3e, 0x3e, 0x3e, 0x3e, 0x3e, 0x00}, /* midi:6 */
	{0x38, 0x10, 0x10, 0x10, 0x10, 0x10, 0x38, 0x00}, /* mode:default */
	{0x38, 0x10, 0x08, 0x04, 0x08, 0x10, 0x38, 0x00}, /* mode: insert */
	{0x00, 0x00, 0x00, 0x82, 0x44, 0x38, 0x00, 0x00}, /* eye open */
	{0x00, 0x38, 0x44, 0x92, 0x28, 0x10, 0x00, 0x00}  /* eye closed */
};

Uint8 font[][8] = {
	{0x00, 0x00, 0x3c, 0x42, 0x42, 0x42, 0x3c, 0x00},
	{0x00, 0x00, 0x30, 0x10, 0x10, 0x10, 0x10, 0x00},
	{0x00, 0x00, 0x7c, 0x02, 0x3c, 0x40, 0x7e, 0x00},
	{0x00, 0x00, 0x7c, 0x02, 0x7c, 0x02, 0x7c, 0x00},
	{0x00, 0x00, 0x12, 0x22, 0x42, 0x7e, 0x02, 0x00},
	{0x00, 0x00, 0x7e, 0x40, 0x3c, 0x02, 0x7e, 0x00},
	{0x00, 0x00, 0x3e, 0x40, 0x7c, 0x42, 0x3c, 0x00},
	{0x00, 0x00, 0x7e, 0x02, 0x04, 0x08, 0x10, 0x00},
	{0x00, 0x00, 0x7e, 0x42, 0x3c, 0x42, 0x7e, 0x00},
	{0x00, 0x00, 0x7e, 0x42, 0x3e, 0x02, 0x02, 0x00},
	{0x00, 0x00, 0x7c, 0x02, 0x3e, 0x42, 0x7a, 0x00},
	{0x00, 0x00, 0x40, 0x40, 0x7c, 0x42, 0x7c, 0x00},
	{0x00, 0x00, 0x00, 0x3e, 0x40, 0x40, 0x3e, 0x00},
	{0x00, 0x00, 0x02, 0x02, 0x3e, 0x42, 0x3e, 0x00},
	{0x00, 0x00, 0x3c, 0x42, 0x7c, 0x40, 0x3e, 0x00},
	{0x00, 0x00, 0x3c, 0x42, 0x70, 0x40, 0x40, 0x00},
	{0x00, 0x00, 0x3e, 0x42, 0x3e, 0x02, 0x7c, 0x00},
	{0x00, 0x00, 0x40, 0x40, 0x7c, 0x42, 0x42, 0x00},
	{0x00, 0x00, 0x10, 0x00, 0x10, 0x10, 0x10, 0x00},
	{0x00, 0x00, 0x7e, 0x04, 0x04, 0x44, 0x38, 0x00},
	{0x00, 0x00, 0x42, 0x44, 0x78, 0x44, 0x42, 0x00},
	{0x00, 0x00, 0x40, 0x40, 0x40, 0x40, 0x3e, 0x00},
	{0x00, 0x00, 0x6c, 0x52, 0x52, 0x52, 0x52, 0x00},
	{0x00, 0x00, 0x5c, 0x62, 0x42, 0x42, 0x42, 0x00},
	{0x00, 0x00, 0x1c, 0x22, 0x42, 0x44, 0x38, 0x00},
	{0x00, 0x00, 0x7c, 0x42, 0x7c, 0x40, 0x40, 0x00},
	{0x00, 0x00, 0x3e, 0x42, 0x3e, 0x02, 0x02, 0x00},
	{0x00, 0x00, 0x5c, 0x62, 0x40, 0x40, 0x40, 0x00},
	{0x00, 0x00, 0x3e, 0x40, 0x3c, 0x02, 0x7c, 0x00},
	{0x00, 0x00, 0x7e, 0x10, 0x10, 0x10, 0x08, 0x00},
	{0x00, 0x00, 0x42, 0x42, 0x42, 0x46, 0x3a, 0x00},
	{0x00, 0x00, 0x42, 0x42, 0x24, 0x24, 0x18, 0x00},
	{0x00, 0x00, 0x42, 0x42, 0x52, 0x52, 0x6c, 0x00},
	{0x00, 0x00, 0x42, 0x42, 0x3c, 0x42, 0x42, 0x00},
	{0x00, 0x00, 0x42, 0x42, 0x3e, 0x02, 0x7c, 0x00},
	{0x00, 0x00, 0x7e, 0x04, 0x18, 0x20, 0x7e, 0x00},
	{0x00, 0x00, 0x3c, 0x42, 0x7e, 0x42, 0x42, 0x00},
	{0x00, 0x00, 0x7c, 0x42, 0x7c, 0x42, 0x7c, 0x00},
	{0x00, 0x00, 0x3e, 0x40, 0x40, 0x40, 0x3e, 0x00},
	{0x00, 0x00, 0x7c, 0x42, 0x42, 0x42, 0x7c, 0x00},
	{0x00, 0x00, 0x7e, 0x40, 0x7e, 0x40, 0x7e, 0x00},
	{0x00, 0x00, 0x7e, 0x40, 0x70, 0x40, 0x40, 0x00},
	{0x00, 0x00, 0x3e, 0x40, 0x5c, 0x42, 0x3e, 0x00},
	{0x00, 0x00, 0x42, 0x42, 0x7e, 0x42, 0x42, 0x00},
	{0x00, 0x00, 0x10, 0x10, 0x10, 0x10, 0x10, 0x00},
	{0x00, 0x00, 0x7e, 0x02, 0x02, 0x42, 0x3c, 0x00},
	{0x00, 0x00, 0x46, 0x48, 0x70, 0x48, 0x46, 0x00},
	{0x00, 0x00, 0x40, 0x40, 0x40, 0x40, 0x7e, 0x00},
	{0x00, 0x00, 0x6e, 0x52, 0x52, 0x52, 0x52, 0x00},
	{0x00, 0x00, 0x62, 0x52, 0x4a, 0x46, 0x42, 0x00},
	{0x00, 0x00, 0x3c, 0x42, 0x42, 0x42, 0x3c, 0x00},
	{0x00, 0x00, 0x7e, 0x42, 0x7c, 0x40, 0x40, 0x00},
	{0x00, 0x00, 0x3c, 0x42, 0x4a, 0x44, 0x3a, 0x00},
	{0x00, 0x00, 0x7e, 0x42, 0x7c, 0x42, 0x42, 0x00},
	{0x00, 0x00, 0x3e, 0x40, 0x7e, 0x02, 0x7c, 0x00},
	{0x00, 0x00, 0x7e, 0x10, 0x10, 0x10, 0x10, 0x00},
	{0x00, 0x00, 0x42, 0x42, 0x42, 0x42, 0x3c, 0x00},
	{0x00, 0x00, 0x42, 0x42, 0x42, 0x24, 0x18, 0x00},
	{0x00, 0x00, 0x52, 0x52, 0x52, 0x52, 0x6e, 0x00},
	{0x00, 0x00, 0x42, 0x24, 0x18, 0x24, 0x42, 0x00},
	{0x00, 0x00, 0x42, 0x24, 0x10, 0x10, 0x10, 0x00},
	{0x00, 0x00, 0x7e, 0x02, 0x3c, 0x40, 0x7e, 0x00},
	{0x00, 0x00, 0x5a, 0x24, 0x42, 0x24, 0x5a, 0x00},
	{0x00, 0x00, 0x24, 0x7e, 0x24, 0x7e, 0x24, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x10, 0x00, 0x10, 0x00, 0x00},
	{0x00, 0x00, 0x66, 0x5a, 0x24, 0x5a, 0x66, 0x00},
	{0x00, 0x00, 0x00, 0x32, 0x42, 0x4c, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x28, 0x00, 0x28, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};

SDL_Window *gWindow = NULL;
SDL_Renderer *gRenderer = NULL;
SDL_Texture *gTexture = NULL;
Uint32 *pixels;
PmStream *midi;

int
clamp(int val, int min, int max)
{
	return (val >= min) ? (val <= max) ? val : max : min;
}

/* misc */

int
getfont(int x, int y, char c, int type, int sel)
{
	if(c >= 'A' && c <= 'Z')
		return c - 'A' + 36;
	if(c >= 'a' && c <= 'z')
		return c - 'a' + 10;
	if(c >= '0' && c <= '9')
		return c - '0';
	if(c == '*')
		return 62;
	if(c == '#')
		return 63;
	if(c == ':')
		return 65;
	if(cursor.x == x && cursor.y == y)
		return 66;
	if(GUIDES) {
		if(x % 8 == 0 && y % 8 == 0)
			return 68;
		if(sel || type || (x % 2 == 0 && y % 2 == 0))
			return 64;
	}
	return 70;
}

int
getstyle(int clr, int type, int sel)
{
	if(sel)
		return clr == 0 ? 4 : 0;
	if(type == 2)
		return clr == 0 ? 0 : 1;
	if(type == 3)
		return clr == 0 ? 1 : 0;
	if(type == 4)
		return clr == 0 ? 0 : 2;
	if(type == 5)
		return clr == 0 ? 2 : 0;
	return clr == 0 ? 0 : 3;
}

void
putpixel(Uint32 *dst, int x, int y, int color)
{
	if(x >= 0 && x < WIDTH - 8 && y >= 0 && y < HEIGHT - 8)
		dst[(y + PAD) * WIDTH + (x + PAD)] = theme[color];
}

void
drawtile(Uint32 *dst, int x, int y, char c, int type, Rect2d *r)
{
	int v, h;
	int sel = x < r->x + r->w && x >= r->x && y < r->y + r->h && y >= r->y;
	Uint8 *icon = font[getfont(x, y, c, type, sel)];
	for(v = 0; v < 8; v++)
		for(h = 0; h < 8; h++) {
			int style = getstyle((icon[v] >> (7 - h)) & 0x1, type, sel);
			putpixel(dst, x * 8 + h, y * 8 + v, style);
		}
}

void
drawicon(Uint32 *dst, int x, int y, Uint8 *icon, int color)
{
	int v, h;
	for(v = 0; v < 8; v++)
		for(h = 0; h < 8; h++) {
			int c = (icon[v] >> (8 - h)) & 0x1;
			putpixel(dst, x + h, y + v, c ? color : 0);
		}
}

void
drawui(Uint32 *dst)
{
	int i, n = 0, bottom = VER * 8 + 8;
	/* CURSOR */
	drawicon(dst, 0 * 8, bottom, font[cursor.x % 36], 1);
	drawicon(dst, 1 * 8, bottom, font[68], 1);
	drawicon(dst, 2 * 8, bottom, font[cursor.y % 36], 1);
	drawicon(dst, 3 * 8, bottom, icons[2], cursor.w > 1 || cursor.h > 1 ? 4 : 3);
	/* FRAME */
	drawicon(dst, 5 * 8, bottom, font[(g.f / 1296) % 36], 1);
	drawicon(dst, 6 * 8, bottom, font[(g.f / 36) % 36], 1);
	drawicon(dst, 7 * 8, bottom, font[g.f % 36], 1);
	drawicon(dst, 8 * 8, bottom, icons[PAUSE ? 1 : 0], PAUSE ? 4 : (g.f - 1) % 8 == 0 ? 2
																					  : 3);
	/* SPEED */
	drawicon(dst, 10 * 8, bottom, font[(BPM / 100) % 10], 1);
	drawicon(dst, 11 * 8, bottom, font[(BPM / 10) % 10], 1);
	drawicon(dst, 12 * 8, bottom, font[BPM % 10], 1);

	for(i = 0; i < VOICES; ++i)
		if(voices[i].length)
			n++;

	if(n > 0)
		drawicon(dst, 13 * 8, bottom, icons[2 + clamp(n, 0, 6)], 2);
	else
		drawicon(dst, 13 * 8, bottom, font[70], 3);
}

void
redraw(Uint32 *dst)
{
	int x, y;
	for(y = 0; y < VER; ++y)
		for(x = 0; x < HOR; ++x)
			drawtile(dst, x, y, get(&g, x, y), gettype(&g, x, y), &cursor);
	drawui(dst);
	SDL_UpdateTexture(gTexture, NULL, dst, WIDTH * sizeof(Uint32));
	SDL_RenderClear(gRenderer);
	SDL_RenderCopy(gRenderer, gTexture, NULL, NULL);
	SDL_RenderPresent(gRenderer);
}

/* midi */

int
nteval(char c)
{
	int sharp, uc, deg, notes[] = {0, 2, 4, 5, 7, 9, 11};
	if(c >= '0' && c <= '9')
		return c - '0';
	sharp = c >= 'a' && c <= 'z';
	uc = sharp ? c - 'a' + 'A' : c;
	deg = uc <= 'B' ? 'G' - 'B' + uc - 'A' : uc - 'C';
	return deg / 7 * 12 + notes[deg % 7] + sharp;
}

Note *
sendmidi(int chn, int val, int vel, int len)
{
	int i = 0;
	for(i = 0; i < VOICES; ++i) {
		Note *n = &voices[i];
		if(!n->length || n->channel != chn || n->value != val)
			continue;
		Pm_WriteShort(midi,
			Pt_Time(),
			Pm_Message(0x90 + n->channel, n->value, 0));
		n->length = 0;
	}
	for(i = 0; i < VOICES; ++i) {
		Note *n = &voices[i];
		if(n->length < 1) {
			n->channel = chn;
			n->value = val;
			n->velocity = vel;
			n->length = len;
			Pm_WriteShort(midi,
				Pt_Time(),
				Pm_Message(0x90 + chn, val, vel * 3));
			return n;
		}
	}
	return NULL;
}

void
parsemidi(char *msg, int msglen)
{
	char chn, oct, nte, vel = 'z', len = '1';
	if(msglen < 3)
		return;
	chn = msg[0];
	oct = msg[1];
	nte = msg[2];
	if(msglen > 3)
		vel = msg[3];
	if(msglen > 4)
		len = msg[4];
	sendmidi(
		cb36(chn),
		12 * cb36(oct) + nteval(nte),
		cb36(vel),
		cb36(len));
}

void
runmsg(void)
{
	int i, j = 0;
	char buf[128];
	/* release */
	for(i = 0; i < VOICES; ++i) {
		Note *n = &voices[i];
		if(n->length > 0) {
			n->length--;
			if(n->length == 0)
				Pm_WriteShort(midi,
					Pt_Time(),
					Pm_Message(0x90 + n->channel, n->value, 0));
		}
	}
	if(g.msglen < 2)
		return;
	/* split messages */
	for(i = 0; i < g.msglen + 1; ++i)
		if(!g.msg[i] || cisp(g.msg[i])) {
			buf[j] = '\0';
			if(j > 0)
				parsemidi(buf, j);
			j = 0;
		} else
			buf[j++] = g.msg[i];
}

void
initmidi(void)
{
	int i;
	Pm_Initialize();
	for(i = 0; i < Pm_CountDevices(); ++i)
		printf("Device #%d -> %s%s\n",
			i,
			Pm_GetDeviceInfo(i)->name,
			i == DEVICE ? "[x]" : "[ ]");
	Pm_OpenOutput(&midi, DEVICE, NULL, 128, 0, NULL, 1);
}

/* options */

int
error(char *msg, const char *err)
{
	printf("Error %s: %s\n", msg, err);
	return 1;
}

void
makedoc(Document *d)
{
	initgrid(&g, HOR, VER);
	redraw(pixels);
}

void
loaddoc(Document *d)
{
}

void
savedoc(Document *d)
{
}

void
select(int x, int y, int w, int h)
{
	Rect2d r;
	r.x = clamp(x, 0, HOR - 1);
	r.y = clamp(y, 0, VER - 1);
	r.w = clamp(w, 1, HOR - x + 1);
	r.h = clamp(h, 1, VER - y + 1);
	if(r.x != cursor.x || r.y != cursor.y || r.w != cursor.w || r.h != cursor.h) {
		cursor = r;
		redraw(pixels);
	}
}

void
reset(void)
{
	MODE = 0;
	GUIDES = 1;
	select(cursor.x, cursor.y, 1, 1);
}

void
modbpm(int mod)
{
	BPM += mod;
	printf("BPM: %d\n", BPM);
	redraw(pixels);
}

void
setplay(int val)
{
	PAUSE = val;
	redraw(pixels);
}

void
setguides(int v)
{
	GUIDES = v;
	redraw(pixels);
}

void
setmode(int v)
{
	MODE = v;
	redraw(pixels);
}

void
comment(Rect2d *r)
{
	int y;
	char c = get(&g, r->x, r->y) == '#' ? '.' : '#';
	for(y = 0; y < r->h; ++y) {
		set(&g, r->x, r->y + y, c);
		set(&g, r->x + r->w - 1, r->y + y, c);
	}
	redraw(pixels);
}

void
move(int x, int y)
{
	select(cursor.x + x, cursor.y + y, cursor.w, cursor.h);
}

void
insert(char c)
{
	int x, y;
	for(x = 0; x < cursor.w; ++x)
		for(y = 0; y < cursor.h; ++y)
			set(&g, cursor.x + x, cursor.y + y, c);
	if(MODE)
		move(1, 0);
	redraw(pixels);
}

void
scale(int w, int h)
{
	if((cursor.w + w) * (cursor.h + h) < CLIPSZ)
		select(cursor.x, cursor.y, cursor.w + w, cursor.h + h);
}

void
frame(void)
{
	rungrid(&g);
	runmsg();
	redraw(pixels);
}

void
selectoption(int option)
{
	switch(option) {
	case 3: select(cursor.x, cursor.y, 1, 1); break;
	case 8:
		setplay(1);
		frame();
		break;
	}
}

void
quit(void)
{
	free(pixels);
	SDL_DestroyTexture(gTexture);
	gTexture = NULL;
	SDL_DestroyRenderer(gRenderer);
	gRenderer = NULL;
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;
	SDL_Quit();
	exit(0);
}

/* clip */

void
copyclip(Rect2d *r, char *c)
{
	int x, y, i = 0;
	for(y = 0; y < r->h; ++y) {
		for(x = 0; x < r->w; ++x) {
			if(i < CLIPSZ - 2)
				c[i++] = get(&g, r->x + x, r->y + y);
		}
		if(i < CLIPSZ - 2)
			c[i++] = '\n';
	}
	c[i] = '\0';
}

void
cutclip(Rect2d *r, char *c)
{
	copyclip(r, c);
	insert('.');
}

void
pasteclip(Rect2d *r, char *c, int insert)
{
	int i = 0, x = r->x, y = r->y;
	char ch;
	while((ch = c[i++])) {
		if(ch == '\n') {
			x = r->x;
			y++;
		} else {
			set(&g, x, y, insert && ch == '.' ? get(&g, x, y) : ch);
			x++;
		}
	}
}

void
moveclip(Rect2d *r, char *c, int x, int y)
{
	copyclip(r, c);
	insert('.');
	move(x, y);
	pasteclip(r, c, 0);
}

/* triggers */

void
domouse(SDL_Event *event)
{
	int cx = event->motion.x / ZOOM / 8;
	int cy = event->motion.y / ZOOM / 8;
	switch(event->type) {
	case SDL_MOUSEBUTTONUP:
		DOWN = 0;
		break;
	case SDL_MOUSEBUTTONDOWN:
		if(cy == VER + 2)
			selectoption(cx - 1);
		else {
			select(cx - 1, cy - 1, 1, 1);
			DOWN = 1;
		}
		break;
	case SDL_MOUSEMOTION:
		if(DOWN)
			select(
				cursor.x,
				cursor.y,
				cx - cursor.x,
				cy - cursor.y);
		break;
	}
}

void
dokey(SDL_Event *event)
{
	int shift = SDL_GetModState() & KMOD_LSHIFT || SDL_GetModState() & KMOD_RSHIFT;
	int ctrl = SDL_GetModState() & KMOD_LCTRL || SDL_GetModState() & KMOD_RCTRL;

	if(ctrl) {
		switch(event->key.keysym.sym) {
		/* Generic */
		case SDLK_n: makedoc(&doc); break;
		case SDLK_r: loaddoc(&doc); break;
		case SDLK_s: savedoc(&doc); break;
		case SDLK_h: setguides(!GUIDES); break;
		/* Edit */
		case SDLK_i: setmode(!MODE); break;
		case SDLK_a: select(0, 0, g.w, g.h); break;
		case SDLK_x: cutclip(&cursor, clip); break;
		case SDLK_c: copyclip(&cursor, clip); break;
		case SDLK_v: pasteclip(&cursor, clip, shift); break;
		case SDLK_UP: moveclip(&cursor, clip, 0, -1); break;
		case SDLK_DOWN: moveclip(&cursor, clip, 0, 1); break;
		case SDLK_LEFT: moveclip(&cursor, clip, -1, 0); break;
		case SDLK_RIGHT: moveclip(&cursor, clip, 1, 0); break;
		case SDLK_SLASH: comment(&cursor); break;
		case SDLK_ESCAPE: reset(); break;
		}
	} else {
		switch(event->key.keysym.sym) {
		case SDLK_PAGEUP: modbpm(1); break;
		case SDLK_PAGEDOWN: modbpm(-1); break;
		case SDLK_UP: shift ? scale(0, -1) : move(0, -1); break;
		case SDLK_DOWN: shift ? scale(0, 1) : move(0, 1); break;
		case SDLK_LEFT: shift ? scale(-1, 0) : move(-1, 0); break;
		case SDLK_RIGHT: shift ? scale(1, 0) : move(1, 0); break;
		case SDLK_SPACE: setplay(!PAUSE); break;
		case SDLK_BACKSPACE: insert('.'); break;
		}
	}
}

void
dotext(SDL_Event *event)
{
	int i;
	for(i = 0; i < SDL_TEXTINPUTEVENT_TEXT_SIZE; ++i) {
		char c = event->text.text[i];
		if(c < ' ' || c > '~')
			break;
		insert(c);
	}
}

int
init(void)
{
	int i, j;
	if(SDL_Init(SDL_INIT_VIDEO) < 0)
		return error("Init", SDL_GetError());
	gWindow = SDL_CreateWindow("Orca",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		WIDTH * ZOOM,
		HEIGHT * ZOOM,
		SDL_WINDOW_SHOWN);
	if(gWindow == NULL)
		return error("Window", SDL_GetError());
	gRenderer = SDL_CreateRenderer(gWindow, -1, 0);
	if(gRenderer == NULL)
		return error("Renderer", SDL_GetError());
	gTexture = SDL_CreateTexture(gRenderer,
		SDL_PIXELFORMAT_ARGB8888,
		SDL_TEXTUREACCESS_STATIC,
		WIDTH,
		HEIGHT);
	if(gTexture == NULL)
		return error("Texture", SDL_GetError());
	pixels = (Uint32 *)malloc(WIDTH * HEIGHT * sizeof(Uint32));
	if(pixels == NULL)
		return error("Pixels", "Failed to allocate memory");
	for(i = 0; i < HEIGHT; i++)
		for(j = 0; j < WIDTH; j++)
			pixels[i * WIDTH + j] = theme[0];
	initmidi();
	return 1;
}

int
main(int argc, char *argv[])
{
	Uint8 tick = 0;

	if(!init())
		return error("Init", "Failure");

	initgrid(&g, HOR, VER);
	select(0, 0, 1, 1);

	if(argc > 1)
		if(!loadgrid(&g, fopen(argv[1], "r")))
			return error("Load", "Failure");

	select(0, 0, 1, 1);

	while(1) {
		SDL_Event event;
		if(!PAUSE) {
			if(tick > 3) {
				frame();
				tick = 0;
			} else
				tick++;
		}
		SDL_Delay(60000 / BPM / 16);
		while(SDL_PollEvent(&event) != 0) {
			switch(event.type) {
			case SDL_QUIT: quit(); break;
			case SDL_MOUSEBUTTONUP:
			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEMOTION: domouse(&event); break;
			case SDL_KEYDOWN: dokey(&event); break;
			case SDL_TEXTINPUT: dotext(&event); break;
			case SDL_WINDOWEVENT:
				if(event.window.event == SDL_WINDOWEVENT_EXPOSED)
					redraw(pixels);
				break;
			}
		}
	}
	quit();
	return 0;
}
