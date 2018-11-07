#pragma once

#include "debug.h"
#include "debug_gl.h"

void glVertexAttribPointer_byname 
(
	GLuint program,
	char const * name,
	uint32_t dim,
	GLenum type,
	GLboolean normalized,
	uint32_t stride,
	uint32_t offset
)
{
	GLint loc = glGetAttribLocation (program, name);
	ASSERT_F (loc >= 0, "glGetAttribLocation (%i, %s) no attribute found", (int) program, name);
	glVertexAttribPointer (loc, (GLint) dim, type, normalized, (GLsizei) stride, (const GLvoid *) (uintptr_t) offset);
	glEnableVertexAttribArray (loc);
}




char const * str_from_gl_primitive (GLenum primitive)
{
	switch (primitive)
	{
		case GL_POINTS:
		return "GL_POINTS";
		case GL_LINES:
		return "GL_LINES";
		case GL_TRIANGLES:
		return "GL_TRIANGLES";
		case GL_LINE_STRIP:
		return "GL_LINE_STRIP";
	}
	return "";
}


void xxgl_layout 
(
	uint32_t n, 
	uint32_t  const index      [],
	uint32_t  const dim        [],
	GLenum    const type       [],
	GLboolean const normalized [],
	uint32_t  const stride     [],
	uint32_t  const offset     [],
	GLenum    const target     [],
	GLuint    const vbo        []
)
{
	for (uint32_t i = 0; i < n; ++ i)
	{
		glBindBuffer (target [i], vbo [i]);
		glEnableVertexAttribArray (index [i]);
		glVertexAttribPointer 
		(
			(GLuint)         index [i], 
			(uint32_t)       dim [i], 
			(GLenum)         type [i], 
			(GLboolean)      normalized [i], 
			(GLint)          stride [i], 
			(GLvoid const *) (uintptr_t) offset [i]
		);
		TRACE_F 
		(
			"%i %i %i %i %i %i", 
			(int)index [i], 
			(int)dim [i], 
			(int)type [i], 
			(int)normalized [i], 
			(int)stride [i], 
			(int)offset [i]
		);
		GL_CHECK_ERROR;
	}
}


void xxgl_allocate 
(
	uint32_t n, 
	GLenum     const   target [],
	GLuint     const   vbo    [],
	uint32_t   const   stride [],
	void       const * data   [],
	GLbitfield const   flags  [],
	uint32_t vn
)
{
	for (uint32_t i = 0; i < n; ++ i)
	{
		GLvoid const * data0 = data ? data [i] : data;
		glBindBuffer (target [i], vbo [i]);
		glBufferStorage (target [i], stride [i] * vn, data0, flags [i]);
		GL_CHECK_ERROR;
	}
}



