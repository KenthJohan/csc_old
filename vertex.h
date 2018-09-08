#pragma once

#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glcorearb.h>
#include <SDL2/SDL.h>
#include <stdio.h>

struct Vertex
{
	float pos [4];
	float col [4];
};


/*
GLuint vertex_get_vao (GLuint program)
{
	GLuint vao;
	glCreateVertexArrays (1, &vao);
	//TODO: This is too unflexiable. Seperate away specific AttribFormat.
	GLuint p = glGetAttribLocation (program, "pos");
	ASSERT_F (p >= 0, "glGetAttribLocation no attribute found.");
	GLuint c = glGetAttribLocation (program, "col");
	ASSERT_F (c >= 0, "glGetAttribLocation no attribute found.");
	glVertexArrayAttribFormat (vao, p, 4, GL_FLOAT, GL_FALSE, (GLuint)offsetof (struct Vertex, pos));
	glVertexArrayAttribFormat (vao, c, 4, GL_FLOAT, GL_FALSE, (GLuint)offsetof (struct Vertex, col));
	glVertexArrayAttribBinding (vao, p, 0);
	glVertexArrayAttribBinding (vao, c, 0);
	glEnableVertexArrayAttrib (vao, p);
	glEnableVertexArrayAttrib (vao, c);
	return vao;
}
*/


void vertex_print (FILE * f, struct Vertex * v)
{
	fprintf (f, "%f %f %f %f : %f %f %f %f\n", v->pos [0], v->pos [1], v->pos [2], v->pos [3], v->col [0], v->col [1], v->col [2], v->col [3]);
	fflush (f);
}
