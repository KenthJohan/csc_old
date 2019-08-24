#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <assert.h>

#include <csc_debug.h>
#include <xgl.h>
#include <csc_sdl.h>
#include <v3.h>
#include <v4.h>
#include <glad/glad.h>

#define WIN_X SDL_WINDOWPOS_UNDEFINED
#define WIN_Y SDL_WINDOWPOS_UNDEFINED
#define WIN_W 640
#define WIN_H 480
#define WIN_TITLE "An SDL2 windows"
#define WIN_N 10

#define APP_QUIT 0x01
#define APP_TEX_COUNT 1

#define APP_RENDER_C          4
#define APP_RENDER_TYPE       GL_UNSIGNED_BYTE
#define APP_RENDER_FORMAT     GL_RGBA
#define APP_RENDER_INTFORMAT  GL_RGBA
#define APP_RENDER_UNIT       0
#define APP_RENDER_MAG_FILTER GL_NEAREST
//#define APP_RENDER_MAG_FILTER GL_LINEAR

#define APP_TEX_W 2
#define APP_TEX_H 2

#define APP_SHADERF "../glhello/test.glfs"
#define APP_SHADERV "../glhello/test.glvs"

struct Application
{
	uint32_t n;
	SDL_Window * window [WIN_N];
};




int main (int argc, char * argv[])
{
	ASSERT (argc);
	ASSERT (argv);

	struct Application a = {0};

	uint32_t flags = 0;
	XSDL_Init (SDL_INIT_VIDEO);
	//a.window [0] = 1;

	XSDL_ASSERT_CreateWindow (a.window [a.n], WIN_TITLE, WIN_X, WIN_Y, WIN_W, WIN_H, SDL_WINDOW_OPENGL);
	SDL_GLContext glcontext = SDL_GL_CreateContext (a.window [a.n]);
	ASSERT (glcontext);
	a.n ++;

	int r = gladLoadGL();
	ASSERT (r);

	GLuint program = glCreateProgram ();XGL_ASSERT_ERROR;
	xgl_attach_shaderfile (program, APP_SHADERF, GL_FRAGMENT_SHADER);
	xgl_attach_shaderfile (program, APP_SHADERV, GL_VERTEX_SHADER);
	glLinkProgram (program);XGL_ASSERT_ERROR;
	glUseProgram (program);XGL_ASSERT_ERROR;
	xgl_program_print (program);

	float vertices [] =
	{   //x      y     s      t
	-1.0f, -1.0f, 0.0f,  1.0f, // BL
	-1.0f,  1.0f, 0.0f,  0.0f, // TL
	1.0f,  1.0f, 1.0f,  0.0f, // TR
	1.0f, -1.0f, 1.0f,  1.0f  // BR
	};
	const GLint indicies [] = {0, 1, 2, 0, 2, 3};

	GLuint vbo;
	GLuint ebo;
	glGenBuffers (1, &(vbo));XGL_ASSERT_ERROR;
	glBindBuffer (GL_ARRAY_BUFFER, vbo);XGL_ASSERT_ERROR;
	glBufferData (GL_ARRAY_BUFFER, sizeof (vertices), vertices, GL_STATIC_DRAW);XGL_ASSERT_ERROR;
	glGenBuffers (1, &(ebo));XGL_ASSERT_ERROR;
	glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, ebo);XGL_ASSERT_ERROR;
	glBufferData (GL_ELEMENT_ARRAY_BUFFER, sizeof (indicies), indicies, GL_STATIC_DRAW);XGL_ASSERT_ERROR;
	{
		GLint pos_attr_loc = glGetAttribLocation (program, "in_Position");
		ASSERT (pos_attr_loc >= 0);
		printf ("pos_attr_loc %i \n", pos_attr_loc);
		glVertexAttribPointer ((GLuint)pos_attr_loc, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)0);
		glEnableVertexAttribArray ((GLuint)pos_attr_loc);
		GLint tex_attr_loc = glGetAttribLocation (program, "in_Texcoord");
		ASSERT (tex_attr_loc >= 0);
		printf ("tex_attr_loc %i \n", tex_attr_loc);
		glVertexAttribPointer ((GLuint)tex_attr_loc, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)(2*sizeof(GLfloat)));
		glEnableVertexAttribArray ((GLuint)tex_attr_loc);
	}
	glUseProgram (program);XGL_ASSERT_ERROR;
	glEnable (GL_BLEND);XGL_ASSERT_ERROR;
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);XGL_ASSERT_ERROR;
	GLuint tex [APP_TEX_COUNT];
	glGenTextures (APP_TEX_COUNT, tex);XGL_ASSERT_ERROR;

	uint8_t img_render [APP_TEX_W * APP_TEX_H * APP_RENDER_C] = {0};
	v4u8_set4 (img_render + APP_RENDER_C * 0, 0xFF, 0xFF, 0xFF, 0xFF);
	v4u8_set4 (img_render + APP_RENDER_C * 1, 0xAA, 0xAA, 0xAA, 0xFF);
	v4u8_set4 (img_render + APP_RENDER_C * 2, 0x66, 0x66, 0x66, 0xFF);
	v4u8_set4 (img_render + APP_RENDER_C * 3, 0x33, 0x33, 0x33, 0xFF);


	glActiveTexture (GL_TEXTURE0 + APP_RENDER_UNIT);XGL_ASSERT_ERROR;
	glBindTexture (GL_TEXTURE_2D, tex [APP_RENDER_UNIT]);XGL_ASSERT_ERROR;
	xgl_uniform1i_set (program, "tex", APP_RENDER_UNIT);
	glTexImage2D (GL_TEXTURE_2D, 0, APP_RENDER_INTFORMAT, APP_TEX_W, APP_TEX_H, 0, APP_RENDER_FORMAT, APP_RENDER_TYPE, img_render);XGL_ASSERT_ERROR;
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);XGL_ASSERT_ERROR;
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);XGL_ASSERT_ERROR;
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, APP_RENDER_MAG_FILTER);XGL_ASSERT_ERROR;
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); XGL_ASSERT_ERROR;
	glGenerateMipmap (GL_TEXTURE_2D);XGL_ASSERT_ERROR;

	SDL_Event event;
	while (1)
	{
		SDL_Delay(1);
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

		if (a.window [0])
		{
			glBindTexture(GL_TEXTURE_2D, tex [0]);XGL_ASSERT_ERROR;
			glTexSubImage2D (GL_TEXTURE_2D, 0, 0, 0, APP_TEX_W, APP_TEX_H, APP_RENDER_FORMAT, APP_RENDER_TYPE, img_render);XGL_ASSERT_ERROR;
			glClear (GL_COLOR_BUFFER_BIT);XGL_ASSERT_ERROR;
			glDrawElements (GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);XGL_ASSERT_ERROR;
			SDL_GL_SwapWindow (a.window [0]);
		}

	}

	XSDL_DestroyWindows (a.window, a.n);
	SDL_Quit();


	return 0;
}
