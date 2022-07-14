#include <stdio.h>
#include <stdlib.h>
#include <math.h>


#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <stdio.h>

int main(int argc, char **argv)
{
	(void)argc, (void)argv;
	if(SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		fprintf(stderr, "SDL_Init error: %s\n", SDL_GetError());
		return 1;
	}
	SDL_Window *windows[2];
	SDL_GLContext contexts[2];

	for(int i = 0; i != sizeof(windows)/sizeof(windows[0]); ++i)
	{
		char title[32];
		snprintf(title, sizeof(title), "window%d", i);
		windows[i] = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		640, 480, SDL_WINDOW_OPENGL);
		if(!windows[i])
		{
			fprintf(stderr, "SDL_CreateWindow error: %s\n", SDL_GetError());
			return 1;
		}
		contexts[i] = SDL_GL_CreateContext(windows[i]);
		if(!contexts[i])
		{
			fprintf(stderr, "SDL_GL_CreateContext error: %s\n", SDL_GetError());
			return 1;
		}
	}

	int running = 1;
	while(running)
	{
		SDL_Event ev;
		while(SDL_PollEvent(&ev))
		{
			if(ev.type == SDL_KEYDOWN && ev.key.keysym.sym == SDLK_q)
			{
				running = 0;
				break;
			}
		}

		SDL_GL_MakeCurrent(windows[0], contexts[0]);
		glClearColor(1, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT);
		SDL_GL_SwapWindow(windows[0]);

		SDL_GL_MakeCurrent(windows[1], contexts[1]);
		glClearColor(0, 1, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT);
		SDL_GL_SwapWindow(windows[1]);
	}

	return 0;
}
