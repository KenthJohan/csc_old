/*
MIT License

Copyright (c) 2019 CSC Project

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once


#include <SDL2/SDL.h>

#define XSDL_Init(x) ASSERT_F(SDL_Init((x)) == 0, "There was an error initializing the SDL library: %s\n", SDL_GetError())
#define XSDL_WaitEvent(x) ASSERT_F(SDL_WaitEvent((x)) == 0, "There was an error while waiting for events: %s\n", SDL_GetError())
#define XSDL_ASSERT_CreateWindow(win, title, x, y, w, h, flags)\
	ASSERT((win) == NULL);\
	(win) = SDL_CreateWindow ((title), (x), (y), (w), (h), (flags));\
	ASSERT_F((win) != NULL, "Could not create window: %s\n", SDL_GetError())


void XSDL_DestroyWindows (SDL_Window * window [], uint32_t n)
{
	for (uint32_t i = 0; i < n; i ++)
	{
		SDL_DestroyWindow (window [i]);
	}
}


