#pragma once

#include "vertex.h"
#include "gbuf.h"
#include "bufq.h"
#include "camera.h"

#define GMESH_DRAW (1 << 0)
#define GMESH_DRAW_ONCE (1 << 1)
#define GMESH_UPDATE (1 << 2)
#define GMESH_UPDATE_ONCE (1 << 3)
#define GMESH_INIT (1 << 4)


struct GMesh
{
	GLuint program;
	GLuint vao;
	GLuint vbo;
	uint32_t flags;
	GLenum mode;
	struct Cam * cam;
	float mm [16];
	void * data;
};


void vao_create_vertex_layout 
(GLuint program, GLuint vao, GLuint vbo, size_t v8, void * v)
{
	GLuint l [2];
	l [0] = glGetAttribLocation (program, "pos");
	l [1] = glGetAttribLocation (program, "col");
	ASSERT_F (l [0] >= 0, "glGetAttribLocation no attribute found.");
	ASSERT_F (l [1] >= 0, "glGetAttribLocation no attribute found.");
	glBindVertexArray (vao);
	glGenBuffers (1, &vbo);
	glBindBuffer (GL_ARRAY_BUFFER, vbo);
	glBufferData (GL_ARRAY_BUFFER, v8, v, GL_STATIC_DRAW);
	glVertexAttribPointer (l [0], 4, GL_FLOAT, GL_FALSE, sizeof (struct Vertex), (void*) offsetof (struct Vertex, pos));
	glEnableVertexAttribArray (l [0]);
	glVertexAttribPointer (l [1], 4, GL_FLOAT, GL_FALSE, sizeof (struct Vertex), (void*) offsetof (struct Vertex, col));
	glEnableVertexAttribArray (l [1]);
}



void gmesh_init (struct GMesh * m, GLuint program)
{
	GBUF_LOOP (size_t, i, m)
	{
		if (!(m [i].flags & GMESH_INIT)) {continue;}
		m [i].flags &= ~GMESH_INIT;
		glGenVertexArrays (1, &m [i].vao);
		glGenBuffers (1, &m [i].vbo);
		vao_create_vertex_layout (program, m [i].vao, m [i].vbo, gbuf_cap8 (m [i].data), m [i].data);
		TRACE_F ("");
		GL_CHECK_ERROR;
	}
}


void gmesh_draw (struct GMesh * m, GLuint uniform_mvp)
{
	GBUF_LOOP (size_t, i, m)
	{
		uint32_t n = gbuf_count (m [i].data);
		//TRACE_F ("%i", n);
		if (n == 0) {continue;}
		if (m [i].flags == 0) {continue;}
		if (m [i].flags & GMESH_DRAW) 
		{
			cam_mvp_update (m [i].cam, m [i].mm, uniform_mvp);
			glBindVertexArray (m [i].vao);
			glDrawArrays (m [i].mode, 0, n);
		}
		else if (m [i].flags & GMESH_DRAW_ONCE)
		{
			m [i].flags &= ~GMESH_DRAW;
		}
		GL_CHECK_ERROR;
	}
}


void gmesh_update (struct GMesh * m)
{
	GBUF_LOOP (size_t, i, m)
	{
		if (m [i].flags == 0) {continue;}
		GLsizeiptr const size8 = gbuf_count (m);
		GLintptr const offset8 = 0;
		if (m [i].flags & GMESH_UPDATE) 
		{
			glBindBuffer (GL_ARRAY_BUFFER, m [i].vbo);
			glBufferSubData (GL_ARRAY_BUFFER, offset8, size8, m [i].data);
			//glNamedBufferSubData (m [i].vbo, offset8, size8, m [i].data);
		}
		if (m [i].flags & GMESH_UPDATE_ONCE) 
		{
			m [i].flags &= ~GMESH_UPDATE;
		}
		GL_CHECK_ERROR;
	}
}









struct Mesh
{
	uint32_t flags;
	
	GLuint program;
	GLuint vao;
	GLuint vbo;
	
	//Draw mode
	GLenum mode;
	
	struct Cam * cam;
	float mm [16];
	
	//
	struct BufQ data;
};



