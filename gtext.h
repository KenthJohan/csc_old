#pragma once

#include <ft2build.h>
#include FT_FREETYPE_H

//Common simple c functions
#include "SDLGL.h"
#include "debug.h"
#include "debug_gl.h"
#include "gen.h"
#include "xxgl.h"


struct gtext_fdim
{
	uint32_t n;
	float * a;
	float * x;
	float * y;
	float * w;
	float * h;
	float * u;
	float * v;
};


void gtext_fdim_calloc (struct gtext_fdim * g)
{
	g->a = calloc (g->n, sizeof (float));
	g->x = calloc (g->n, sizeof (float));
	g->y = calloc (g->n, sizeof (float));
	g->w = calloc (g->n, sizeof (float));
	g->h = calloc (g->n, sizeof (float));
	g->u = calloc (g->n, sizeof (float));
	g->v = calloc (g->n, sizeof (float));
	ASSERT (g->a);
	ASSERT (g->x);
	ASSERT (g->y);
	ASSERT (g->w);
	ASSERT (g->h);
	ASSERT (g->u);
	ASSERT (g->v);
}


void gtext_setup 
(
	GLuint tex, 
	FT_Face face, 
	struct gtext_fdim * g
)
{
	GLenum const target = GL_TEXTURE_2D_ARRAY;
	GLsizei const width = 50;
	GLsizei const height = 50;
	GLsizei const layerCount = 128;
	GLsizei const mipLevelCount = 1;
	GLenum const internalformat = GL_R8;
	glBindTexture (target, tex);
	glTexStorage3D (target, mipLevelCount, internalformat, width, height, layerCount);
	GL_CHECK_ERROR;
	
	//IMPORTANT. Disable byte-alignment restriction.
	glPixelStorei (GL_UNPACK_ALIGNMENT, 1);
	
	for (uint8_t c = 0; c < 128; c++)
	{
		{
			int r = FT_Load_Char (face, c, FT_LOAD_RENDER);
			ASSERT_F (r == 0, "ERROR::FREETYTPE: Failed to load Glyph %i", r);
			if (r) {continue;}
		}
		
		g->a [c] = (float) (face->glyph->advance.x >> 6);
		g->x [c] = (float)face->glyph->bitmap_left;
		g->y [c] = (float)face->glyph->bitmap_top - (float)face->glyph->bitmap.rows;
		g->w [c] = (float)face->glyph->bitmap.width;
		g->h [c] = (float)face->glyph->bitmap.rows;
		g->u [c] = (float)face->glyph->bitmap.width / (float)width;
		g->v [c] = (float)face->glyph->bitmap.rows / (float)height;
		
		GLint xoffset = 0;
		GLint yoffset = 0;
		GLint zoffset = c;
		
		//TRACE_F ("glyph %i %i", face->glyph->bitmap.width, face->glyph->bitmap.rows);
		
		glTexSubImage3D 
		(
			target, 
			0, 
			xoffset, 
			yoffset, 
			zoffset, 
			face->glyph->bitmap.width,
			face->glyph->bitmap.rows,
			1, 
			GL_RED,
			GL_UNSIGNED_BYTE, 
			face->glyph->bitmap.buffer
		);
		GL_CHECK_ERROR;
	}
	
	// Always set reasonable texture parameters
	glTexParameteri (target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri (target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri (target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri (target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri (target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri (target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri (target, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri (target, GL_TEXTURE_MAX_LEVEL, 0);
	//Single channel textures stores pixel values in the red channel.
	//It is better to store it in the alpha channel because 
	//then we can choose the color from the vertex color.
	//https://www.g-truc.net/post-0734.html
	//https://www.khronos.org/opengl/wiki/GLAPI/glTexParameter
	glTexParameteri (target, GL_TEXTURE_SWIZZLE_A, GL_RED);
	glTexParameteri (target, GL_TEXTURE_SWIZZLE_R, GL_ZERO);
}

















void gtext_draw 
(
	char const * text, 
	struct gtext_fdim * g,
	struct xxgl_drawrange * dr,
	uint32_t di,
	
	float ox,
	float oy,
	float sx,
	float sy
)
{
	GLenum const vtarget = GL_ARRAY_BUFFER;
	GLbitfield const vaccess = GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT;
	uint32_t const vdim = 4;
	uint32_t const vsize = sizeof (float) * vdim;
	GLintptr const voffset8 = dr->offset [di] * vsize;
	GLsizeiptr const vlength8 = dr->length [di] * vsize;
	
	//Vertex buffer for position, texpos, color.
	float * v;
	//Iterator and current character of the text.
	char const * c;
	
	//Set the positions of each character of the text in the position vertex buffer.
	// vertex buffer: |xyzw, xyzw, xyzw, xyzw, xyzw, xywz|xyzw, xyzw, xyzw, xyzw, xyzw, xywz|
	// vertex buffer: |triangle1,        triangle2       |triangle1,        triangle2       |
	// vertex buffer: |square = 24*floats                |square = 24*floats                |
	glBindBuffer (vtarget, dr->vbo [VBO_POS]);
	v = glMapBufferRange (vtarget, voffset8, vlength8, vaccess);
	ASSERT (v);
	
	c = text;
	float ax = 0.0f;
	while (*c)
	{
		uint8_t i = *c;
		float x = sx * g->x [i] + ox + ax;
		float y = sy * g->y [i] + oy;
		float w = sx * g->w [i];
		float h = sy * g->h [i];
		gen_square_pos (v, x, y, w, h);
		v += 24;
		ax += g->a [i];
		c ++;
	}
	glUnmapBuffer (vtarget);
	
	
	glBindBuffer (vtarget, dr->vbo [VBO_TEX]);
	v = glMapBufferRange (vtarget, voffset8, vlength8, vaccess);
	ASSERT (v);
	c = text;
	while (*c)
	{
		uint8_t i = *c;
		gen4x6_square_tex1 (v, g->u [i], g->v [i], i);
		v += 24;
		c ++;
	}
	glUnmapBuffer (vtarget);
}
