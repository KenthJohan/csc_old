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




char const * xxgl_primitive2str (GLenum primitive)
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


char const * xxgl_type2str (GLenum type)
{
	switch (type)
	{
		case GL_FLOAT:
		return "GL_FLOAT";
	}
	return "";
}


char const * xxgl_bool2str (GLboolean value)
{
	switch (value)
	{
		case GL_TRUE:
		return "GL_TRUE";
		case GL_FALSE:
		return "GL_FALSE";
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
			"index: %02i; dim: %01i; type: %10s; normalized: %8s; stride: %03i; offset: %04i;", 
			(int)index [i], 
			(int)dim [i], 
			xxgl_type2str (type [i]), 
			xxgl_bool2str (normalized [i]), 
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


enum vbo_index
{
	VBO_POS,
	VBO_COL,
	VBO_TEX,
	VBO_N
};


struct xxgl_drawrange
{
	uint32_t n;
	uint32_t * offset;
	uint32_t * length;
	uint32_t * capacity;
	GLenum * primitive;
	
	GLuint vbo [3];
};


float * xxgl_vf32_map (struct xxgl_drawrange * dr, uint32_t idr, uint32_t ivbo, uint32_t vdim)
{
	ASSERT (dr);
	GLenum const vtarget = GL_ARRAY_BUFFER;
	GLbitfield const vaccess = GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT;
	uint32_t const vsize = sizeof (float) * vdim;
	GLintptr const voffset8 = dr->offset [idr] * vsize;
	GLsizeiptr const vlength8 = dr->length [idr] * vsize;
	glBindBuffer (vtarget, dr->vbo [ivbo]);
	float * v = glMapBufferRange (vtarget, voffset8, vlength8, vaccess);
	ASSERT (v);
	return v;
}


void xxgl_unmap (struct xxgl_drawrange * dr)
{
	ASSERT (dr);
	GLenum const vtarget = GL_ARRAY_BUFFER;
	glUnmapBuffer (vtarget);
}


void xxgl_drawrange_calloc (struct xxgl_drawrange * dr)
{
	dr->offset = calloc (dr->n, sizeof (uint32_t));
	dr->length = calloc (dr->n, sizeof (uint32_t));
	dr->capacity = calloc (dr->n, sizeof (uint32_t));
	dr->primitive = calloc (dr->n, sizeof (GLenum));
	ASSERT (dr->offset);
	ASSERT (dr->length);
	ASSERT (dr->capacity);
	ASSERT (dr->primitive);
	glGenBuffers (3, dr->vbo);
	//ASSERT (glIsBuffer (dr->vbo [0]));
	//ASSERT (glIsBuffer (dr->vbo [1]));
	//ASSERT (glIsBuffer (dr->vbo [2]));
}


uint32_t xxgl_drawrange_cap (struct xxgl_drawrange * dr)
{
	return dr->offset [dr->n-1] + dr->capacity [dr->n-1];
}


//first: Specifies the starting index in the enabled arrays.
//count: Specifies the number of indices to be rendered.
void xxgl_drawrange_draw (struct xxgl_drawrange * dr, uint32_t i)
{
	glDrawArrays (dr->primitive [i], (GLint)dr->offset [i], (GLsizei)dr->length [i]);
	//glDrawArrays (GL_LINES, (GLint)first [i], (GLsizei)count [i]);
}





void xxgl_dr_v4f32_repeat4
(
	struct xxgl_drawrange * dr,
	uint32_t ivbo,
	uint32_t idr,
	float x0,
	float x1,
	float x2,
	float x3
)
{
	float * v = xxgl_vf32_map (dr, idr, ivbo, 4);
	v4f32_repeat4 (dr->length [idr], v, x0, x1, x2, x3);
	xxgl_unmap (dr);
}


void xxgl_dr_v4f32_grid
(
	struct xxgl_drawrange * dr,
	uint32_t ivbo,
	uint32_t idr,
	uint32_t w,
	uint32_t h
)
{
	GLenum const vtarget = GL_ARRAY_BUFFER;
	GLbitfield const vaccess = GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT;
	uint32_t const vdim = 4;
	uint32_t const vsize = sizeof (float) * vdim;
	GLintptr const voffset8 = dr->offset [idr] * vsize;
	GLsizeiptr const vlength8 = dr->length [idr] * vsize;
	
	ASSERT (glIsBuffer (dr->vbo [ivbo]));
	glBindBuffer (vtarget, dr->vbo [ivbo]);
	float * v = glMapBufferRange (vtarget, voffset8, vlength8, vaccess);
	ASSERT (v);
	gen4x6_grid_pos (v, w, h);
	glUnmapBuffer (vtarget);
}


void xxgl_dr_v4f32_randomcolor 
(
	struct xxgl_drawrange * dr,
	uint32_t ivbo,
	uint32_t idr,
	uint32_t n
)
{
	GLenum const vtarget = GL_ARRAY_BUFFER;
	GLbitfield const vaccess = GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT;
	uint32_t const vdim = 4;
	uint32_t const vsize = sizeof (float) * vdim;
	GLintptr const voffset8 = dr->offset [idr] * vsize;
	GLsizeiptr const vlength8 = dr->length [idr] * vsize;
	ASSERT (glIsBuffer (dr->vbo [ivbo]));
	glBindBuffer (vtarget, dr->vbo [ivbo]);
	float * v = glMapBufferRange (vtarget, voffset8, vlength8, vaccess);
	ASSERT (v);
	for (uint32_t i = 0; i < n; ++i)
	{
		float r = (float)rand () / (float)RAND_MAX;
		float g = (float)rand () / (float)RAND_MAX;
		float b = (float)rand () / (float)RAND_MAX;
		float a = 1.0f;
		v4f32_repeat4 (6, v, r, g, b, a);
		v += vdim * 6;
	}
	glUnmapBuffer (vtarget);
}



void xxgl_dr_v4f32_squaretex_countup 
(
	struct xxgl_drawrange * dr,
	uint32_t ivbo,
	uint32_t idr,
	uint32_t n,
	uint32_t a
)
{
	GLenum const vtarget = GL_ARRAY_BUFFER;
	GLbitfield const vaccess = GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT;
	uint32_t const vdim = 4;
	uint32_t const vsize = sizeof (float) * vdim;
	GLintptr const voffset8 = dr->offset [idr] * vsize;
	GLsizeiptr const vlength8 = dr->length [idr] * vsize;
	ASSERT (glIsBuffer (dr->vbo [ivbo]));
	glBindBuffer (vtarget, dr->vbo [ivbo]);
	float * v = glMapBufferRange (vtarget, voffset8, vlength8, vaccess);
	ASSERT (v);
	for (uint32_t i = 0; i < n; ++i)
	{
		gen4x6_square_tex1 (v, 1.0f, 1.0f, a);
		a ++;
		v += 24;
	}
	glUnmapBuffer (vtarget);
}

