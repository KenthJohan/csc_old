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