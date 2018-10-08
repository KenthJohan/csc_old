#pragma once

#include "debug.h"
#include "debug_gl.h"
#include "glwrap.h"

struct Mesh
{
	GLenum mode;     //GL_TRIANGLE etc...
	uint32_t vfirst; //vertex first
	uint32_t vcount; //vertex count
};


void mesh_add (struct Mesh * m, GLint * vfirst, size_t vcount, GLenum primitive)
{
	m->mode = primitive;
	m->vfirst = (*vfirst);
	m->vcount = vcount;
	(*vfirst) += vcount;
}


void mesh_draw_one (struct Mesh * m)
{
	GLenum const mode = m->mode;
	GLint const vfirst = m->vfirst;
	GLsizei const vcount = m->vcount;
	//TRACE_F ("%x %i %i", mode, (int)vfirst, (int)vcount);
	glDrawArrays (m->mode, m->vfirst, m->vcount);
}


void mesh_draw (size_t n, struct Mesh m [])
{
	for (size_t i = 0; i < n; ++ i)
	{
		mesh_draw_one (m + i);
	}
}


void * mesh_add_glMapBufferRange 
(
	struct Mesh * m, 
	GLint * vfirst,
	GLuint vbo, 
	size_t vcount, 
	size_t dim, 
	GLenum primitive,
	GLenum target
)
{
	GLintptr const offset = (*vfirst) * sizeof (float) * dim;
	GLsizeiptr const length = vcount * dim * sizeof (float);
	GLbitfield const access = GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT;
	mesh_add (m, vfirst, vcount, primitive);
	glBindBuffer (target, vbo);
	return glMapBufferRange (target, offset, length, access);
}


void mesh_add_square (struct Mesh * m, GLint * vfirst, GLuint vbo)
{
	/*
	float v [] =
	{
		 0.5f,  0.5f, 1.0f, 1.0f, // top right
		 0.5f, -0.5f, 1.0f, 0.0f, // bottom right
		-0.5f,  0.5f, 0.0f, 0.0f, // top left 
		
		 0.5f, -0.5f, 1.0f, 0.0f, // bottom right
		-0.5f, -0.5f, 0.0f, 0.0f, // bottom left
		-0.5f,  0.5f, 0.0f, 1.0f  // top left 
	};
	*/
	float v [] =
	{
		-1.0f,  1.0f, 0.0f, 0.0f, // top left
		 1.0f,  1.0f, 0.0f, 1.0f, // top right
		 1.0f, -1.0f, 1.0f, 1.0f, // bottom right 
		
		 1.0f, -1.0f, 1.0f, 1.0f, // bottom right 
		-1.0f, -1.0f, 1.0f, 0.0f, // bottom left
		-1.0f,  1.0f, 0.0f, 0.0f, // top left
	};
	
	size_t const dim = 4;
	size_t const vcount = 6;
	GLenum const primitive = GL_TRIANGLES;
	GLenum const target = GL_ARRAY_BUFFER;
	void * buffer = mesh_add_glMapBufferRange (m, vfirst, vbo, vcount, dim, primitive, target);
	memcpy (buffer, v, sizeof (v));
	glUnmapBuffer (target);
}


void mesh_add_plot 
(
	struct Mesh * m, 
	GLint * vfirst, 
	GLuint vbo, 
	size_t n, 
	float y [], 
	GLenum primitive
)
{
	size_t const dim = 4;
	size_t const vcount = n;
	GLenum const target = GL_ARRAY_BUFFER;
	float * v = (float *) mesh_add_glMapBufferRange (m, vfirst, vbo, vcount, dim, primitive, target);
	for (size_t j = 0; j < n; ++ j)
	{
		v [j*dim + 0] = ((float)j / ((float)n)) - 0.5f;
		v [j*dim + 1] = y [j];
		v [j*dim + 2] = 0.0f;
		v [j*dim + 3] = 1.0f;
	}
	glUnmapBuffer (target);
}


void mesh_add_line 
(
	struct Mesh * m, 
	GLint * vfirst, 
	GLuint vbo, 
	float x0, 
	float y0, 
	float x1, 
	float y1
)
{
	size_t const dim = 4;
	size_t const vcount = 2;
	GLenum const primitive = GL_LINES;
	GLenum const target = GL_ARRAY_BUFFER;
	float * v = (float *) mesh_add_glMapBufferRange (m, vfirst, vbo, vcount, dim, primitive, target);
	v [0] = x0;
	v [1] = y0;
	v [2] = 0.0f;
	v [3] = 1.0f;
	v [4] = x1;
	v [5] = y1;
	v [6] = 0.0f;
	v [7] = 1.0f;
	glUnmapBuffer (target);
}





