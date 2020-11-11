#include <SDL2/SDL.h>
#include <stdio.h>
#include "sim.h"

#define HOR 32
#define VER 16
#define PAD 8
#define ZOOM 1
#define color1 0x000000
#define color2 0x72DEC2
#define color3 0x888888
#define color4 0xFFFFFF
#define color0 0x222222

#define PLIMIT 256
#define SZ (HOR * VER * 16)

typedef struct {
	int x, y;
} Point2d;

typedef struct {
	int x, y, w, h;
} Rect2d;

unsigned char font[80];
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

void
drawtile(uint32_t *dst, int x, int y, char c, int type)
{
	int v, h;
	int target = 0;
	for(v = 0; v < 8; v++)
		for(h = 7; h >= 0; h--) {
			int px = (x * 8) + h;
			int py = (y * 8) + v;
			if(x == selection.x && y == selection.y)
				dst[(py + PAD) * WIDTH + (px + PAD)] = colors[1];
			else {
				int ch1 = font[v];
				int clr = ((ch1 >> x) & 0x1);
				/* printf("%d ", ch1); */
				dst[(py + PAD) * WIDTH + (px / 8 + PAD)] = colors[clr];
			}
		}
}

void
draw(uint32_t *dst)
{
	int x, y;
	for(y = 0; y < VER; ++y) {
		for(x = 0; x < HOR; ++x) {
			drawtile(dst, x, y, get(&g, x, y), gettype(&g, x, y));
		}
	}

	/*
	int b, i, j, id = 0;
	for(b = 0; b < SZ; b += 16) {
		int ti = id / 64;
		int tx = ti % HOR;
		int ty = ti / HOR;
		for(i = 0; i < 8; i++)
			for(j = 7; j >= 0; j--) {
				int ch1 = chrbuf[b + i];
				int ch2 = chrbuf[b + i + 8];
				int color = ((ch1 >> j) & 0x1) + (((ch2 >> j) & 0x1) << 1);
				int odd = (ti + (ti / HOR + 2)) % 2 == 0;
				int px = (ti / (HOR * VER)) * (8 * HOR) + tx * 8 + (id % 8);
				int py = ((ti / HOR) * 8) + ((id % 64) / 8);
				if(tx == selection.x && ty == selection.y)
					dst[(py + PAD) * WIDTH + (px + PAD)] = colors[1];
				else
					dst[(py + PAD) * WIDTH + (px + PAD)] = colors[GUIDES && odd && color == 0 ? 4 : color];
				id++;
			}
	}
	*/
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
	int reqdraw = 0;
	if((x < 0 && selection.x > 0) || (x > 0 && selection.x < HOR - 1)) {
		selection.x += x;
		reqdraw = 1;
	}
	if((y < 0 && selection.y > 0) || (y > 0 && selection.y < VER - 1)) {
		selection.y += y;
		reqdraw = 1;
	}
	if(reqdraw)
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
			(event->motion.x - (PAD * ZOOM)) / ZOOM,
			(event->motion.y - (PAD * ZOOM)) / ZOOM),
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
	switch(event->key.keysym.sym) {
	case SDLK_s:
		set(&g, selection.x, selection.y, 'S');
		break;
	case SDLK_0:
		set(&g, selection.x, selection.y, '0');
		break;
	case SDLK_UP:
		move(0, -1);
		printf("up\n");
		break;
	case SDLK_DOWN:
		move(0, 1);
		printf("down\n");
		break;
	case SDLK_LEFT:
		move(-1, 0);
		printf("left\n");
		break;
	case SDLK_RIGHT:
		move(1, 0);
		printf("right\n");
		break;
	}
	/* update(); */
}

int
init(void)
{
	int i, j;
	if(SDL_Init(SDL_INIT_VIDEO) < 0)
		return error("Init", SDL_GetError());
	gWindow = SDL_CreateWindow("Toy",
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
	int i;
	FILE *f = fopen("font.chr", "rb");
	if(f == NULL)
		return error("Font", "Invalid input file");
	if(!fread(font, sizeof(font), 1, f))
		return error("Font", "Invalid input size");
	for(i = 0; i < 10; ++i) {
		printf("%d\n", font[i]);
	}
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
	set(&g, 3, 6, 'A');
	set(&g, 4, 6, 'a');
	set(&g, 7, 2, '0');
	set(&g, 8, 7, '0');

	run(&g);
	print(&g);

	select(0, 0, 0, 0);
	draw(pixels);

	while(1) {
		int tick = SDL_GetTicks();
		SDL_Event event;
		if(tick < ticknext)
			SDL_Delay(ticknext - tick);
		ticknext = tick + (1000 / FPS);

		if(tickrun == 10) {
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
