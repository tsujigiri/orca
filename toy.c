#include <SDL2/SDL.h>
#include <stdio.h>
#include "sim.h"

#define HOR 32
#define VER 16
#define PAD 8
#define ZOOM 2
#define color1 0x000000
#define color2 0x72DEC2
#define color3 0xFFFFFF
#define color4 0x444444
#define color0 0xffb545

#define PLIMIT 256
#define SZ (HOR * VER * 16)

typedef struct {
	int x, y;
} Point2d;

typedef struct {
	int x, y, w, h;
} Rect2d;

unsigned char font[1200];
int colors[] = {color1, color2, color3, color4, color0};
int WIDTH = 8 * HOR + PAD * 2;
int HEIGHT = 8 * VER + PAD * 2;
int FPS = 30;
int GUIDES = 1;
int COLOR = 3;
SDL_Window *gWindow = NULL;
SDL_Renderer *gRenderer = NULL;
SDL_Texture *gTexture = NULL;
uint32_t *pixels;

Rect2d selection;

Grid g;

Point2d
Pt2d(int x, int y)
{
	Point2d p;
	p.x = x;
	p.y = y;
	return p;
}

Point2d
clampt(Point2d p, int step)
{
	p.x = abs((p.x + step / 2) / step) * step;
	p.y = abs((p.y + step / 2) / step) * step;
	return p;
}

int
clamp(int val, int min, int max)
{
	return (val >= min) ? (val <= max) ? val : max : min;
}

int
selected(int x, int y)
{
	return x < selection.x + selection.w && x >= selection.x && y < selection.y + selection.h && y >= selection.y;
}

void
insert(char c)
{
	set(&g, selection.x, selection.y, c);
}

/* misc */

int
guide(int x, int y)
{
	if(!GUIDES)
		return 0;
	if(x % 32 == 0 && y % 32 == 0)
		return 3;
	else if(x % 8 == 0 && y % 8 == 0)
		return 4;
	return 0;
}

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
	if(c == '.' && type)
		return 64;
	if(c == '.' && !sel)
		return 70;
	if(selection.x == x && selection.y == y)
		return 66;
	if(sel)
		return 64;
	return 70;
}

int
getstyle(int clr, int type, int sel)
{
	if(sel)
		return colors[clr == 0 ? 4 : 0];
	if(type == 2)
		return colors[clr == 0 ? 0 : 1];
	if(type == 3)
		return colors[clr == 0 ? 1 : 0];
	if(type == 4)
		return colors[clr == 0 ? 0 : 2];
	if(type == 5)
		return colors[clr == 0 ? 2 : 0];
	return colors[clr == 0 ? 0 : 3];
}

void
drawtile(uint32_t *dst, int x, int y, char c, int type)
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
			dst[key] = getstyle(clr, type, sel);
		}
}

void
draw(uint32_t *dst)
{
	int x, y;
	for(y = 0; y < VER; ++y)
		for(x = 0; x < HOR; ++x)
			drawtile(dst, x, y, get(&g, x, y), gettype(&g, x, y));
	SDL_UpdateTexture(gTexture, NULL, dst, WIDTH * sizeof(uint32_t));
	SDL_RenderClear(gRenderer);
	SDL_RenderCopy(gRenderer, gTexture, NULL, NULL);
	SDL_RenderPresent(gRenderer);
}

void
select(int x, int y, int w, int h)
{
	selection.x = x;
	selection.y = y;
	selection.w = w;
	selection.h = h;
	draw(pixels);
}

void
move(int x, int y)
{
	selection.x += x;
	selection.y += y;
	selection.x = clamp(selection.x, 0, HOR);
	selection.y = clamp(selection.y, 0, VER);
	draw(pixels);
}

void
scale(int w, int h)
{
	selection.w += w;
	selection.h += h;
	selection.w = clamp(selection.w, 1, 8);
	selection.h = clamp(selection.h, 1, 8);
	draw(pixels);
}

int
error(char *msg, const char *err)
{
	printf("Error %s: %s\n", msg, err);
	return 1;
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

void
render(void)
{
	draw(pixels);
}

void
domouse(SDL_Event *event)
{
	Point2d touch = clampt(
		Pt2d(
			(event->motion.x - (PAD * ZOOM) - (8 * ZOOM / 2)) / ZOOM,
			(event->motion.y - (PAD * ZOOM) - (8 * ZOOM / 2)) / ZOOM),
		8);
	switch(event->type) {
	case SDL_MOUSEBUTTONUP:
		printf("mouse-up\n");
		break;
	case SDL_MOUSEBUTTONDOWN:
		printf("%d,%d\n", touch.x / 8, touch.y / 8);
		select(touch.x / 8, touch.y / 8, 1, 1);
		printf("mouse-down\n");
		break;
	case SDL_MOUSEMOTION:
		break;
	}
}

void
dokey(SDL_Event *event)
{
	int shift = SDL_GetModState() & KMOD_LSHIFT || SDL_GetModState() & KMOD_RSHIFT;
	switch(event->key.keysym.sym) {
	case SDLK_BACKSPACE: insert('.'); break;
	case SDLK_ASTERISK: insert('*'); break;
	case SDLK_HASH: insert('#'); break;
	case SDLK_PERIOD: insert('.'); break;
	case SDLK_COLON: insert(':'); break;
	case SDLK_SEMICOLON: insert(':'); break;
	case SDLK_ESCAPE: select(selection.x, selection.y, 1, 1); break;
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
	case SDLK_a: insert(shift ? 'A' : 'a'); break;
	case SDLK_b: insert(shift ? 'B' : 'b'); break;
	case SDLK_c: insert(shift ? 'C' : 'c'); break;
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
	case SDLK_v: insert(shift ? 'V' : 'v'); break;
	case SDLK_w: insert(shift ? 'W' : 'w'); break;
	case SDLK_x: insert(shift ? 'X' : 'x'); break;
	case SDLK_y: insert(shift ? 'Y' : 'y'); break;
	case SDLK_z: insert(shift ? 'Z' : 'z'); break;
	case SDLK_UP: shift ? scale(0, -1) : move(0, -1); break;
	case SDLK_DOWN: shift ? scale(0, 1) : move(0, 1); break;
	case SDLK_LEFT: shift ? scale(-1, 0) : move(-1, 0); break;
	case SDLK_RIGHT: shift ? scale(1, 0) : move(1, 0); break;
	}
	/* update(); */
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
	pixels = (uint32_t *)malloc(WIDTH * HEIGHT * sizeof(uint32_t));
	if(pixels == NULL)
		return error("Pixels", "Failed to allocate memory");
	for(i = 0; i < HEIGHT; i++)
		for(j = 0; j < WIDTH; j++)
			pixels[i * WIDTH + j] = color1;
	return 1;
}

int
loadfont(void)
{
	FILE *f = fopen("font.chr", "rb");
	if(f == NULL)
		return error("Font", "Invalid input file");
	if(!fread(font, sizeof(font), 1, f))
		return error("Font", "Invalid input size");
	fclose(f);
	return 1;
}

int
main(int argc, char *argv[])
{
	int ticknext = 0;
	int tickrun = 0;

	if(!init())
		return error("Init", "Failure");
	if(!loadfont())
		return error("Font", "Failure");

	create(&g, HOR, VER);
	set(&g, 3, 3, 'S');
	set(&g, 3, 6, 'B');
	set(&g, 4, 6, '2');
	set(&g, 7, 2, '0');
	set(&g, 8, 7, '0');
	run(&g);

	select(0, 0, 1, 1);
	draw(pixels);

	while(1) {
		int tick = SDL_GetTicks();
		SDL_Event event;
		if(tick < ticknext)
			SDL_Delay(ticknext - tick);
		ticknext = tick + (1000 / FPS);

		if(tickrun == 8) {
			run(&g);
			draw(pixels);
			tickrun = 0;
		}
		tickrun++;

		while(SDL_PollEvent(&event) != 0) {
			if(event.type == SDL_QUIT)
				quit();
			else if(event.type == SDL_MOUSEBUTTONUP ||
					event.type == SDL_MOUSEBUTTONDOWN ||
					event.type == SDL_MOUSEMOTION) {
				domouse(&event);
			} else if(event.type == SDL_KEYDOWN)
				dokey(&event);
			else if(event.type == SDL_WINDOWEVENT)
				if(event.window.event == SDL_WINDOWEVENT_EXPOSED)
					draw(pixels);
		}
	}
	quit();
	return 0;
}
