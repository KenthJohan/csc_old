#pragma once

#include <SDL2/SDL.h>

#ifdef _WIN32
#include <GL/glew.h>
#endif


#ifdef linux
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glcorearb.h>
#endif

#include "debug.h"


SDL_GLContext SDL_GL_CreateContext_CC (SDL_Window * window)
{
	SDL_GLContext context = SDL_GL_CreateContext (window);
	ASSERT_F (context != NULL, "SDL_GL_CreateContext: %s", SDL_GetError());
	#ifdef _WIN32
	{
		glewExperimental = GL_TRUE;
		GLenum err = glewInit ();
		ASSERT (err == GLEW_OK);
	}
	#endif
	return context;
}
	
	
	

