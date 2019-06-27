#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <assert.h>
#include <csc_debug.h>

#define WIN_X SDL_WINDOWPOS_UNDEFINED
#define WIN_Y SDL_WINDOWPOS_UNDEFINED
#define WIN_W 640
#define WIN_H 480
#define WIN_TITLE "An SDL2 windows"
#define WIN_N 10

#define APP_QUIT 0x01

struct Application
{
	uint32_t n;
	SDL_Window * window [WIN_N];
};

void XSDL_DestroyWindows (SDL_Window * window [], uint32_t n)
{
	for (uint32_t i = 0; i < n; i ++)
	{
		SDL_DestroyWindow (window [i]);
	}
}

#define XSDL_Init(x) ASSERT_F(SDL_Init((x)) == 0, "There was an error initializing the SDL library: %s\n", SDL_GetError())
#define XSDL_WaitEvent(x) ASSERT_F(SDL_WaitEvent((x)) == 0, "There was an error while waiting for events: %s\n", SDL_GetError())
#define XSDL_ASSERT_CreateWindow(win, title, x, y, w, h, flags)\
	ASSERT((win) == NULL);\
	(win) = SDL_CreateWindow ((title), (x), (y), (w), (h), (flags));\
	ASSERT_F((win) != NULL, "Could not create window: %s\n", SDL_GetError())


int main (int argc, char * argv[])
{
	ASSERT (argc);
	ASSERT (argv);

	struct Application a = {0};

	uint32_t flags = 0;
	XSDL_Init (SDL_INIT_VIDEO);
	//a.window [0] = 1;

	XSDL_ASSERT_CreateWindow (a.window [a.n], WIN_TITLE, WIN_X, WIN_Y, WIN_W, WIN_H, SDL_WINDOW_OPENGL);
	a.n ++;


	SDL_Event event;
	while (1)
	{
		if (flags & APP_QUIT) {break;}
		SDL_WaitEvent (&event);
		//XSDL_WaitEvent (&event);
		//TRACE_F ("%i", event.type);
		switch (event.type)
		{
		case SDL_WINDOWEVENT:
			switch (event.window.event)
			{
			case SDL_WINDOWEVENT_CLOSE:
				SDL_DestroyWindow (SDL_GetWindowFromID (event.window.windowID));
				a.n --;
				a.window [a.n] = NULL;
				break;
			case SDL_WINDOWEVENT_SIZE_CHANGED:
				break;
			}
			break;
		case SDL_KEYDOWN:
			switch (event.key.keysym.sym)
			{
			case SDLK_ESCAPE:
				SDL_DestroyWindow (SDL_GetWindowFromID (event.window.windowID));
				a.n --;
				a.window [a.n] = NULL;
				break;

			case SDLK_c:
				printf ("SDL_CreateWindow\n");
				if (a.n < WIN_N)
				{

					XSDL_ASSERT_CreateWindow (a.window [a.n], WIN_TITLE, WIN_X, WIN_Y, WIN_W, WIN_H, SDL_WINDOW_OPENGL);
					a.n ++;
					//TRACE_F ("id:%i", SDL_GetWindowID (a.window [0]));
					//TRACE_F ("id:%i", SDL_GetWindowID (a.window [a.n]));
				}
				break;

			default:
				break;
			}
			break;

		case SDL_QUIT:
			flags |= APP_QUIT;
			break;

		case SDL_MOUSEMOTION:
			//printf ("%i %i\n", event.motion.x, event.motion.y);
			break;
		}
	}

	XSDL_DestroyWindows (a.window, a.n);
	SDL_Quit();


	return 0;
}
