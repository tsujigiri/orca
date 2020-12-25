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
#define SZ (HOR * VER * 16)

typedef unsigned char Uint8;

#define PLIMIT 256
#define SZ (HOR * VER * 16)
#define DEVICE 0

typedef struct {
	int x, y, w, h;
} Rect2d;

typedef struct {
	int channel, value, velocity, length;
} Note;

char OCTAVE[] = {'C', 'c', 'D', 'd', 'E', 'F', 'f', 'G', 'g', 'A', 'a', 'B'};

Rect2d cursor;
Grid g;

int WIDTH = 8 * HOR + PAD * 2;
int HEIGHT = 8 * (VER + 2) + PAD * 2;
int FPS = 30, DOWN = 0, ZOOM = 2, PAUSE = 0;

Uint32 theme[] = {
	0x000000,
	0xFFFFFF,
	0x72DEC2,
	0x666666,
	0xffb545};

Uint8 icons[][8] = {
	{0x38, 0x7c, 0xfe, 0xfe, 0xfe, 0x7c, 0x38, 0x00},
	{0x38, 0x44, 0x82, 0x82, 0x82, 0x44, 0x38, 0x00},
	{0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x00},
	{0x06, 0x18, 0x20, 0x40, 0x40, 0x80, 0x80, 0x00},
	{0x02, 0x02, 0x04, 0x38, 0x40, 0x80, 0x80, 0x00},
	{0xfe, 0x82, 0x82, 0x82, 0x82, 0x82, 0xfe, 0x00},
	{0x1e, 0x06, 0x0a, 0x12, 0x20, 0x40, 0x80, 0x00},
	{0x06, 0x18, 0x22, 0x40, 0x42, 0x80, 0xaa, 0x00},
	{0x00, 0x00, 0x00, 0x82, 0x44, 0x38, 0x00, 0x00}, /* eye open */
	{0x00, 0x38, 0x44, 0x92, 0x28, 0x10, 0x00, 0x00}  /* eye closed */
};

char clip[1024];
Uint8 font[1200];
Note playing[16];

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

int
selected(int x, int y)
{
	return x < cursor.x + cursor.w && x >= cursor.x && y < cursor.y + cursor.h && y >= cursor.y;
}

void
insert(char c)
{
	int x, y;
	for(x = 0; x < cursor.w; ++x)
		for(y = 0; y < cursor.h; ++y)
			set(&g, cursor.x + x, cursor.y + y, c);
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
	if(x % 8 == 0 && y % 8 == 0)
		return 68;
	if(cursor.x == x && cursor.y == y)
		return 66;
	if(sel || type || (x % 2 == 0 && y % 2 == 0))
		return 64;
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
drawtile(Uint32 *dst, int x, int y, char c, int type)
{
	int v, h;
	int sel = selected(x, y);
	int offset = getfont(x, y, c, type, sel) * 8 * 2;
	for(v = 0; v < 8; v++)
		for(h = 0; h < 8; h++) {
			int px = (x * 8) + (8 - h);
			int py = (y * 8) + v;
			int ch1 = font[offset + v];
			int ch2 = font[offset + v + 8];
			int clr = ((ch1 >> h) & 0x1) + ((ch2 << h) & 0x1);
			int key = (py + PAD) * WIDTH + (px + PAD);
			dst[key] = theme[getstyle(clr, type, sel)];
		}
}

int
keypixel(int x, int y)
{
	return (y + PAD) * WIDTH + (x + PAD);
}

void
putpixel(Uint32 *dst, int x, int y, int color)
{
	if(x >= 0 && x < WIDTH - 8 && y >= 0 && y < HEIGHT - 8)
		dst[keypixel(x, y)] = theme[color];
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
	int bottom = VER * 8 + 8;
	drawicon(dst, 0, bottom, icons[PAUSE == 1 ? 4 : 0], 1);
}

void
redraw(Uint32 *dst)
{
	int x, y;
	for(y = 0; y < VER; ++y)
		for(x = 0; x < HOR; ++x)
			drawtile(dst, x, y, get(&g, x, y), gettype(&g, x, y));
	drawui(dst);
	SDL_UpdateTexture(gTexture, NULL, dst, WIDTH * sizeof(Uint32));
	SDL_RenderClear(gRenderer);
	SDL_RenderCopy(gRenderer, gTexture, NULL, NULL);
	SDL_RenderPresent(gRenderer);
}

/* options */

int
error(char *msg, const char *err)
{
	printf("Error %s: %s\n", msg, err);
	return 1;
}

void
modzoom(int mod)
{
	if((mod > 0 && ZOOM < 4) || (mod < 0 && ZOOM > 1)) {
		ZOOM += mod;
		SDL_SetWindowSize(gWindow, WIDTH * ZOOM, HEIGHT * ZOOM);
	}
}

void
setplay(int val)
{
	PAUSE = val;
	redraw(pixels);
}

void
select(int x, int y, int w, int h)
{
	cursor.x = clamp(x, 0, HOR - 1);
	cursor.y = clamp(y, 0, VER - 1);
	cursor.w = clamp(w, 1, 36);
	cursor.h = clamp(h, 1, 36);
	redraw(pixels);
}

void
move(int x, int y)
{
	select(cursor.x + x, cursor.y + y, cursor.w, cursor.h);
}

void
scale(int w, int h)
{
	select(cursor.x, cursor.y, cursor.w + w, cursor.h + h);
}

void
selectoption(int option)
{
	switch(option) {
	case 0: setplay(!PAUSE); break;
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
		for(x = 0; x < r->w; ++x)
			if(i < 1024)
				c[i++] = get(&g, r->x + x, r->y + y);
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
pasteclip(Rect2d *r, char *c)
{
	int i = 0, x = r->x, y = r->y;
	char ch;
	while((ch = c[i++])) {
		if(ch == '\n') {
			x = r->x;
			y++;
		} else
			set(&g, x++, y, ch);
	}
}

void
moveclip(Rect2d *r, char *c, int x, int y)
{
	copyclip(r, c);
	insert('.');
	move(x, y);
	pasteclip(r, c);
}

/* midi */

int
nteval(int o, char c)
{
	switch(c) {
	case 'A': return 9;
	case 'a': return 10;
	case 'B': return 11;
	case 'b':
	case 'C': return 0;
	case 'c': return 1;
	case 'D': return 2;
	case 'd': return 3;
	case 'E': return 4;
	case 'e':
	case 'F': return 5;
	case 'f': return 6;
	case 'G': return 7;
	case 'g': return 8;
	default:
		if(c >= '0' && c <= '9')
			return c - '0';
		else
			return nteval(o + 12, c - 7);
	}
	return 0;
}

Note *
sendmidi(int chn, int val, int vel, int len)
{
	int i = 0;
	for(i = 0; i < 16; ++i) {
		Note *n = &playing[i];
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
		base36(chn),
		12 * base36(oct) + nteval(0, nte),
		base36(vel),
		base36(len));
}

void
runmsg(void)
{
	int i, j = 0;
	char buf[128];
	/* release */
	for(i = 0; i < 16; ++i) {
		Note *n = &playing[i];
		if(n->length > 0) {
			n->length--;
			if(n->length == 0)
				Pm_WriteShort(midi,
					Pt_Time(),
					Pm_Message(0x90 + n->channel, n->value, 0));
		}
	}
	/* split messages */
	for(i = 0; i < g.msglen + 1; ++i)
		if(!g.msg[i] || cisp(g.msg[i])) {
			buf[j] = '\0';
			parsemidi(buf, j);
			j = 0;
		} else
			buf[j++] = g.msg[i];
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
	switch(event->key.keysym.sym) {
	case SDLK_EQUALS:
	case SDLK_PLUS: modzoom(1); break;
	case SDLK_UNDERSCORE:
	case SDLK_MINUS: modzoom(-1); break;
	case SDLK_UP:
		if(ctrl)
			moveclip(&cursor, clip, 0, -1);
		else if(shift)
			scale(0, -1);
		else
			move(0, -1);
		break;
	case SDLK_DOWN:
		if(ctrl)
			moveclip(&cursor, clip, 0, 1);
		else if(shift)
			scale(0, 1);
		else
			move(0, 1);
		break;
	case SDLK_LEFT:
		if(ctrl)
			moveclip(&cursor, clip, -1, 0);
		else if(shift)
			scale(-1, 0);
		else
			move(-1, 0);
		break;
	case SDLK_RIGHT:
		if(ctrl)
			moveclip(&cursor, clip, 1, 0);
		else if(shift)
			scale(1, 0);
		else
			move(1, 0);
		break;
	case SDLK_SPACE: setplay(!PAUSE); break;
	case SDLK_BACKSPACE: insert('.'); break;
	case SDLK_ASTERISK: insert('*'); break;
	case SDLK_HASH: insert('#'); break;
	case SDLK_PERIOD: insert('.'); break;
	case SDLK_COLON: insert(':'); break;
	case SDLK_SEMICOLON: insert(':'); break;
	case SDLK_ESCAPE: select(cursor.x, cursor.y, 1, 1); break;
	case SDLK_0: insert('0'); break;
	case SDLK_1: insert('1'); break;
	case SDLK_2: insert('2'); break;
	case SDLK_3: insert(shift ? '#' : '3'); break;
	case SDLK_4: insert('4'); break;
	case SDLK_5: insert('5'); break;
	case SDLK_6: insert('6'); break;
	case SDLK_7: insert('7'); break;
	case SDLK_8: insert('8'); break;
	case SDLK_9: insert('9'); break;
	case SDLK_a: ctrl ? select(0, 0, g.w, g.h) : insert(shift ? 'A' : 'a'); break;
	case SDLK_b: insert(shift ? 'B' : 'b'); break;
	case SDLK_c: ctrl ? copyclip(&cursor, clip) : insert(shift ? 'C' : 'c'); break;
	case SDLK_d: insert(shift ? 'D' : 'd'); break;
	case SDLK_e: insert(shift ? 'E' : 'e'); break;
	case SDLK_f: insert(shift ? 'F' : 'f'); break;
	case SDLK_g: insert(shift ? 'G' : 'g'); break;
	case SDLK_h: insert(shift ? 'H' : 'h'); break;
	case SDLK_i: insert(shift ? 'I' : 'i'); break;
	case SDLK_j: insert(shift ? 'J' : 'j'); break;
	case SDLK_k: insert(shift ? 'K' : 'k'); break;
	case SDLK_l: insert(shift ? 'L' : 'l'); break;
	case SDLK_m: insert(shift ? 'M' : 'm'); break;
	case SDLK_n: insert(shift ? 'N' : 'n'); break;
	case SDLK_o: insert(shift ? 'O' : 'o'); break;
	case SDLK_p: insert(shift ? 'P' : 'p'); break;
	case SDLK_q: insert(shift ? 'Q' : 'q'); break;
	case SDLK_r: insert(shift ? 'R' : 'r'); break;
	case SDLK_s: insert(shift ? 'S' : 's'); break;
	case SDLK_t: insert(shift ? 'T' : 't'); break;
	case SDLK_u: insert(shift ? 'U' : 'u'); break;
	case SDLK_v: ctrl ? pasteclip(&cursor, clip) : insert(shift ? 'V' : 'v'); break;
	case SDLK_w: insert(shift ? 'W' : 'w'); break;
	case SDLK_x: ctrl ? cutclip(&cursor, clip) : insert(shift ? 'X' : 'x'); break;
	case SDLK_y: insert(shift ? 'Y' : 'y'); break;
	case SDLK_z: insert(shift ? 'Z' : 'z'); break;
	}
}

void
initmidi(void)
{
	int i;
	Pm_Initialize();
	for(i = 0; i < Pm_CountDevices(); ++i) {
		char const *name = Pm_GetDeviceInfo(i)->name;
		printf("Device #%d -> %s%s\n", i, name, i == DEVICE ? "[x]" : "[ ]");
	}
	Pm_OpenOutput(&midi, DEVICE, NULL, 128, 0, NULL, 1);
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
loadfont(void)
{
	FILE *f = fopen("font.chr", "rb");
	if(f == NULL)
		return error("Font", "Invalid font file");
	if(!fread(font, sizeof(font), 1, f))
		return error("Font", "Invalid font size");
	fclose(f);
	return 1;
}

int
main(int argc, char *argv[])
{
	int ticknext = 0, tickrun = 0;

	if(!init())
		return error("Init", "Failure");
	if(!loadfont())
		return error("Font", "Failure");

	initgrid(&g, HOR, VER);
	select(0, 0, 1, 1);

	if(argc > 1)
		if(!loadgrid(&g, fopen(argv[1], "r")))
			return error("Load", "Failure");

	select(0, 0, 1, 1);

	while(1) {
		int tick = SDL_GetTicks();
		SDL_Event event;
		if(tick < ticknext)
			SDL_Delay(ticknext - tick);
		ticknext = tick + (1000 / FPS);

		if(!PAUSE && tickrun >= 8) {
			rungrid(&g);
			runmsg();
			redraw(pixels);
			tickrun = 0;
		}
		tickrun++;

		while(SDL_PollEvent(&event) != 0) {
			if(event.type == SDL_QUIT)
				quit();
			else if(event.type == SDL_MOUSEBUTTONUP ||
					event.type == SDL_MOUSEBUTTONDOWN ||
					event.type == SDL_MOUSEMOTION)
				domouse(&event);
			else if(event.type == SDL_KEYDOWN)
				dokey(&event);
			else if(event.type == SDL_WINDOWEVENT)
				if(event.window.event == SDL_WINDOWEVENT_EXPOSED)
					redraw(pixels);
		}
	}
	quit();
	return 0;
}
