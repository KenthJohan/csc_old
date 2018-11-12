#pragma once

#include <SDL2/SDL.h>

#ifdef _WIN32
#define GLEW_STATIC
#include <GL/glew.h>
#endif


#ifdef __linux__
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glcorearb.h>
#endif

#include "debug.h"


SDL_GLContext SDL_GL_CreateContext_CC (SDL_Window * window)
{
	SDL_GL_SetAttribute (SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute (SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute (SDL_GL_CONTEXT_MINOR_VERSION, 3);
	
	TRACE_F ("Using OpenGL version                        : %s", glGetString (GL_VERSION));
	TRACE_F ("Using OpenGL Shader Language (GLSL) version : %s", glGetString (GL_SHADING_LANGUAGE_VERSION));
	
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
	
	
	

