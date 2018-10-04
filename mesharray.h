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


struct Mesharray
{
	//Capacity of the (meshes).
	//The (n) must never be higher than this.
	size_t cap;
	
	//Current number of meshes.
	//This will be incremented when adding a mesh.
	size_t n;
	
	//The pointer to the (meshes) memory.
	struct Mesh * meshes;
};


void mesharray_init (struct Mesharray * m, size_t cap)
{
	m->cap = cap;
	m->n = 0;
	m->meshes = calloc (cap + 1, sizeof (struct Mesh));
}


void mesharray_draw_one (struct Mesharray * m, size_t i)
{
	GLenum const mode = m->meshes [i].mode;
	GLint const vfirst = m->meshes [i].vfirst;
	GLsizei const vcount = m->meshes [i].vcount;
	//TRACE_F ("%x %i %i", mode, (int)vfirst, (int)vcount);
	glDrawArrays (mode, vfirst, vcount);
}


void mesharray_draw_all (struct Mesharray * m)
{
	for (size_t i = 0; i < m->n; ++ i)
	{
		mesharray_draw_one (m, i);
	}
}


//Add a mesh.
//With the number of vertices (vcount) in the mesh.
//With the (primitive) in the mesh.
//The (primitive) can be GL_POINTS, GL_TRIANGLES, etc...
struct Mesh * mesharray_add (struct Mesharray * m, size_t vcount, GLenum primitive)
{
	TRACE_F ("Add mesh : %4iv %s", (int)vcount, str_from_gl_primitive (primitive));
	ASSERT (m->n < m->cap);
	struct Mesh * mesh = m->meshes + m->n;
	mesh [0].mode = primitive;
	mesh [0].vcount = vcount;
	mesh [1].vfirst = mesh [0].vfirst + mesh [0].vcount;
	m->n ++;
	return mesh;
}


//Duplicate the draw vertex (index and count).
//Calling the draw function will draw the same vertices as previous mesh.
//The primitive can be different.
struct Mesh * mesharray_dup (struct Mesharray * m, GLenum primitive)
{
	ASSERT (m->n >= 1);
	ASSERT (m->n < m->cap);
	struct Mesh * mesh = m->meshes + m->n;
	mesh [0].mode = primitive;
	mesh [0].vcount = mesh [-1].vcount;
	mesh [0].vfirst = mesh [-1].vfirst;
	mesh [1].vfirst = mesh [0].vfirst + mesh [0].vcount;
	m->n ++;
	return mesh;
}


//Uses glMapBufferRange.

void * mesharray_add_glMapBufferRange 
(
	struct Mesharray * m, 
	GLuint vbo, 
	size_t vcount, 
	size_t dim, 
	GLenum primitive,
	GLenum target
)
{
	struct Mesh * mesh = mesharray_add (m, vcount, primitive);
	GLintptr const offset = mesh->vfirst * sizeof (float) * dim;
	GLsizeiptr const length = mesh->vcount * dim * sizeof (float);
	GLbitfield const access = GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT;
	glBindBuffer (target, vbo);
	return glMapBufferRange (target, offset, length, access);
}


void mesharray_add_square (struct Mesharray * m, GLuint vbo)
{
	float v [] =
	{
		 0.5f,  0.5f, 1.0f, 1.0f, // top right
		 0.5f, -0.5f, 1.0f, 0.0f, // bottom right
		-0.5f,  0.5f, 0.0f, 0.0f, // top left 
		
		 0.5f, -0.5f, 1.0f, 0.0f, // bottom right
		-0.5f, -0.5f, 0.0f, 0.0f, // bottom left
		-0.5f,  0.5f, 0.0f, 1.0f  // top left 
	};
	
	size_t const dim = 4;
	size_t const vcount = 6;
	GLenum const primitive = GL_TRIANGLES;
	GLenum const target = GL_ARRAY_BUFFER;
	void * buffer = mesharray_add_glMapBufferRange (m, vbo, vcount, dim, primitive, target);
	memcpy (buffer, v, sizeof (v));
	glUnmapBuffer (target);
}


void mesharray_add_plot (struct Mesharray * m, GLuint vbo, size_t n, float y [], GLenum primitive)
{
	size_t const dim = 4;
	size_t const vcount = n;
	GLenum const target = GL_ARRAY_BUFFER;
	float * v = mesharray_add_glMapBufferRange (m, vbo, vcount, dim, primitive, target);
	for (size_t i = 0; i < n; ++ i)
	{
		v [i*dim + 0] = ((float)i / ((float)n)) - 0.5f;
		v [i*dim + 1] = y [i];
		v [i*dim + 2] = 0.0f;
		v [i*dim + 3] = 1.0f;
	}
	glUnmapBuffer (target);
}


void mesharray_add_line (struct Mesharray * m, GLuint vbo, float x0, float y0, float x1, float y1)
{
	size_t const dim = 4;
	size_t const vcount = 2;
	GLenum const primitive = GL_LINES;
	GLenum const target = GL_ARRAY_BUFFER;
	float * v = mesharray_add_glMapBufferRange (m, vbo, vcount, dim, primitive, target);
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





