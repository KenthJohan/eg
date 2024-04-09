#include <SDL2/SDL.h>
#include <stdio.h>
#include "renderer.h"
#include "microui.h"
#include "microui_sdl.h"
#include "demo_window.h"



typedef struct {
	mu_Context muctx;
	int dummy;
} muext_context_t;




static int text_width(mu_Font font, const char *text, int len)
{
	if (len == -1) {
		len = strlen(text);
	}
	return r_get_text_width(text, len);
}

static int text_height(mu_Font font)
{
	return r_get_text_height();
}

int main(int argc, char **argv)
{
	SDL_Init(SDL_INIT_EVERYTHING);
	r_init("CAN GUI");
	muext_context_t *ctx = malloc(sizeof(muext_context_t));
	mu_init((mu_Context*)ctx);
	((mu_Context*)ctx)->text_width = text_width;
	((mu_Context*)ctx)->text_height = text_height;

	for (;;) {
		SDL_Event e;
		while (SDL_PollEvent(&e)) {
			mu_backend_events((mu_Context*)ctx, &e);
		}
		demo_window_progress((mu_Context*)ctx);
		mu_backend_render((mu_Context*)ctx);
	}

	return 0;
}
