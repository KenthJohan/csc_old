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


struct GLBA
{
	size_t n;
	GLuint * vbo;
	GLenum * target;
	size_t * vesize8;
	void ** data;
};


void glba_init (struct GLBA * ba)
{
	ba->vbo     = calloc (ba->n, sizeof (GLuint));
	ba->target  = calloc (ba->n, sizeof (GLenum));
	ba->vesize8 = calloc (ba->n, sizeof (size_t));
	ba->data    = calloc (ba->n, sizeof (void *));
	glGenBuffers (ba->n, ba->vbo);
}


void glba_update (struct GLBA * ba, size_t i)
{
	GLenum const target    = ba->target [i];
	GLintptr const offset8 = ba->offset8 [i];
	GLsizeiptr const size8 = ba->size8 [i];
	GLvoid const * data    = ba->data [i];
	glBufferSubData (target, offset8, size8, data);
}



struct GLVA
{
	float * PVM;
	GLuint vao;
	//GLuint vbo;
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
	//ASSERT (glIsBuffer (va->vbo) == GL_TRUE);
	ASSERT (glIsVertexArray (va->vao) == GL_TRUE);
	ASSERT (va->PVM != NULL); //Must have MVP matrix?
	//ASSERT (vcount >= 0);
	//ASSERT (esize8 >= 0);
	ASSERT (va->meshes.p != NULL);
}


void glva_draw (struct GLVA * va)
{
	//Specifies the name of the vertex array to bind.
	//Installs a program object as part of current rendering state.
	//Set the model view projectio matrix.
	glBindVertexArray (va->vao);
	glUseProgram (va->program);
	glUniformMatrix4fv (va->uniform_mvp, 1, GL_FALSE, va->PVM);
	
	BUFV_LOOP (va->meshes, struct GLMesh *, m)
	{
		//Check if the mesh (m) need to be drawn.
		uint32_t const flags = m->flags;
		if (!(flags & (MESH_DRAW_ALLWAYS | MESH_DRAW_ONCE))) {continue;}
		if (flags & MESH_DRAW_ONCE) {m->flags &= ~MESH_DRAW_ONCE;}
		
		//Renaming variables to shorter names.
		//Draw the bound vertex array.
		//mode  : Specifies what kind of primitives to render. E.g. GL_TRIANGLES, GL_LINES, GL_POINTS, etc.
		//first : Specifies the starting index in the bound arrays. 
		//count : Specifies the number of indices to be rendered. 
		GLenum const mode = m->mode;
		GLint const first = m->first;
		GLsizei const count = m->count;
		glDrawArrays (mode, first, count);
	}
}


/*
void glva_update (struct GLVA * va)
{
	//Renaming the variables to shorter names.
	//n : Number of meshes.
	//p : Pointer to the array of meshes.
	size_t const n = va->meshes.n;
	struct GLMesh * m = va->meshes.p;
	
	//Binding the GL_ARRAY_BUFFER will 
	//set OpenGL global state GL_ARRAY_BUFFER binding buffer.
	//Loading GPU data will be done on the GL_ARRAY_BUFFER binding buffer.
	GLenum const target = GL_ARRAY_BUFFER;
	glBindBuffer (target, va->vbo);
	
	
	for (size_t i = 0; i < n; ++ i)
	{
		//Check if the mesh need to be updated.
		uint32_t const flags = m [i].flags;
		if (!(flags & (MESH_UPDATE_ALLWAYS | MESH_UPDATE_ONCE))) {continue;}
		if (flags & MESH_UPDATE_ONCE) {m [i].flags &= ~MESH_UPDATE_ONCE;}
		
		//
		GLintptr const offset8 = m [i].first * va->esize8;
		GLsizeiptr const size8 = m [i].count * va->esize8;
		GLvoid * data = m [i].data;
		typedef long long int lli;
		TRACE_F ("mesh %lli : glBufferSubData : Offset : %10lli B. Size : %10lli B.", (lli)i, (lli)offset8, (lli)size8);
		glBufferSubData (target, offset8, size8, data);
	}
}
*/
