#pragma once

#include "debug.h"
#include "debug_gl.h"
#include "bufv.h"



#define MESH_ERROR            1 << 0
#define MESH_UPDATE_ONCE      1 << 1
#define MESH_UPDATE_ALLWAYS   1 << 2
#define MESH_DRAW_ONCE        1 << 3
#define MESH_DRAW_ALLWAYS     1 << 4


struct GLMesh
{
	uint32_t flags;
	GLenum mode;
	GLint first;
	GLsizei count;
	void * data;
};


struct GLVA
{
	GLuint vao;
	GLuint vbo;
	GLuint program;
	
	struct BUFV (struct GLMesh, meshes);
	//size_t meshes_i;
	//size_t meshes_n;
	//struct Mesh * meshes;
};


void glva_init (struct GLVA * va, GLuint program, size_t n, void * data)
{
	size_t const size8 = n * sizeof (struct Vertex);
	glGenVertexArrays (1, &va->vao);
	glGenBuffers (1, &va->vbo);
	GLuint l [2];
	l [0] = glGetAttribLocation (program, "pos");
	l [1] = glGetAttribLocation (program, "col");
	ASSERT_F (l [0] >= 0, "glGetAttribLocation no attribute found.");
	ASSERT_F (l [1] >= 0, "glGetAttribLocation no attribute found.");
	glBindVertexArray (va->vao);
	glBindBuffer (GL_ARRAY_BUFFER, va->vbo);
	glBufferData (GL_ARRAY_BUFFER, size8, data, GL_DYNAMIC_DRAW);
	glVertexAttribPointer (l [0], 4, GL_FLOAT, GL_FALSE, sizeof (struct Vertex), (void*) offsetof (struct Vertex, p));
	glEnableVertexAttribArray (l [0]);
	glVertexAttribPointer (l [1], 4, GL_FLOAT, GL_FALSE, sizeof (struct Vertex), (void*) offsetof (struct Vertex, c));
	glEnableVertexAttribArray (l [1]);
}


void glva_draw (struct GLVA * va)
{
	//size_t const n = va->meshes.n;
	//struct Mesh * m = va->meshes.p;
	glBindVertexArray (va->vao);
	
	BUFV_LOOP (va->meshes, struct GLMesh *, e)
	//for (size_t i = 0; i < n; ++ i)
	{
		//uint32_t const flags = m [i].flags;
		uint32_t const flags = e->flags;
		if (!(flags & (MESH_DRAW_ALLWAYS | MESH_DRAW_ONCE))) {continue;}
		//if (flags & MESH_DRAW_ONCE) {m [i].flags &= ~MESH_DRAW_ONCE;}
		if (flags & MESH_DRAW_ONCE) {e->flags &= ~MESH_DRAW_ONCE;}
		//GLenum const mode = m [i].mode;
		GLenum const mode = e->mode;
		//GLint const first = m [i].first;
		GLint const first = e->first;
		//GLsizei const count = m [i].count;
		GLsizei const count = e->count;
		glDrawArrays (mode, first, count);
	}
}


void glva_update (struct GLVA * va)
{
	size_t const n = va->meshes.n;
	struct GLMesh * m = va->meshes.p;
	GLenum const target = GL_ARRAY_BUFFER;
	glBindBuffer (target, va->vbo);
	for (size_t i = 0; i < n; ++ i)
	{
		uint32_t const flags = m [i].flags;
		if (!(flags & (MESH_UPDATE_ALLWAYS | MESH_UPDATE_ONCE))) {continue;}
		if (flags & MESH_UPDATE_ONCE) {m [i].flags &= ~MESH_UPDATE_ONCE;}
		GLintptr const offset8 = m [i].first * sizeof (struct Vertex);
		GLsizeiptr const size8 = m [i].count * sizeof (struct Vertex);
		GLvoid * data = m [i].data;
		TRACE_F ("mesh %i : glBufferSubData : %i B", i, size8);
		glBufferSubData (target, offset8, size8, data);
	}
}
