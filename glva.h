#pragma once

#include "debug.h"
#include "debug_gl.h"
#include "bufv.h"



#define MESH_ERROR            1U << 0
#define MESH_UPDATE_ONCE      1U << 1
#define MESH_UPDATE_ALLWAYS   1U << 2
#define MESH_DRAW_ONCE        1U << 3
#define MESH_DRAW_ALLWAYS     1U << 4


struct GLMesh
{
	uint32_t flags;
	GLenum mode;
	GLint first;
	//GLsizei count;
	size_t count;
	void * data;
};


struct GLVA
{
	float * PVM;
	GLuint vao;
	GLuint vbo;
	GLuint program;
	GLuint uniform_mvp;
	size_t vcount;
	size_t esize8;
	struct BUFV (struct GLMesh, meshes);
};


void glva_init (struct GLVA * va)
{
	//The struct should be configured first.
	//Note 1: Functions argument does not show names but struct member does.
	//Note 2: Configuring the whole struct first gives a better summery of what is going on.
	
	ASSERT (glIsProgram (va->program) == GL_TRUE);
	ASSERT (glIsBuffer (va->vbo) == GL_TRUE);
	ASSERT (glIsVertexArray (va->vao) == GL_TRUE);
	ASSERT (va->PVM != NULL); //Must have MVP matrix?
	//ASSERT (vcount >= 0);
	//ASSERT (esize8 >= 0);
	ASSERT (va->meshes.p != NULL);
}


void glva_draw (struct GLVA * va)
{
	glBindVertexArray (va->vao);
	glUseProgram (va->program);
	glUniformMatrix4fv (va->uniform_mvp, 1, GL_FALSE, va->PVM);
	BUFV_LOOP (va->meshes, struct GLMesh *, e)
	{
		uint32_t const flags = e->flags;
		if (!(flags & (MESH_DRAW_ALLWAYS | MESH_DRAW_ONCE))) {continue;}
		if (flags & MESH_DRAW_ONCE) {e->flags &= ~MESH_DRAW_ONCE;}
		GLenum const mode = e->mode;
		GLint const first = e->first;
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
		GLintptr const offset8 = m [i].first * va->esize8;
		GLsizeiptr const size8 = m [i].count * va->esize8;
		GLvoid * data = m [i].data;
		typedef long long int lli;
		TRACE_F ("mesh %lli : glBufferSubData : Offset : %10lli B. Size : %10lli B.", (lli)i, (lli)offset8, (lli)size8);
		glBufferSubData (target, offset8, size8, data);
	}
}
