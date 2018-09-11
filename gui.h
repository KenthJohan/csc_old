#pragma once

#include "v.h"
#include "v2.h"


__attribute__((packed))
struct GUI_Vertex
{
	float p [2];
	float c [4];
};


void gui_vertex_print (struct GUI_Vertex * v, size_t n)
{
	for (size_t i = 0; i < 8; ++ i)
	{
		printf ("%f %f : %f %f %f %f\n", v [i].p [0], v [i].p [1], v [i].c [0], v [i].c [1], v [i].c [2], v [i].c [3]);
	}
}


__attribute__((packed))
struct GUI_VertexRectangle
{
	struct GUI_Vertex sw;
	struct GUI_Vertex nw;
	struct GUI_Vertex ne;
	struct GUI_Vertex se;
};


void gui_vertex_layout 
(GLuint vao, GLuint vbo, GLuint ebo, size_t v8, size_t e8, void * v, void * e)
{
	glBindVertexArray (vao);
	glBindBuffer (GL_ARRAY_BUFFER, vbo);
	glBufferData (GL_ARRAY_BUFFER, v8, v, GL_STATIC_DRAW);
	glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData (GL_ELEMENT_ARRAY_BUFFER, e8, e, GL_STATIC_DRAW);
	glVertexAttribPointer (0, 2, GL_FLOAT, GL_FALSE, sizeof (struct GUI_Vertex), (void*)0);
	glEnableVertexAttribArray (0);
	glVertexAttribPointer (1, 4, GL_FLOAT, GL_FALSE, sizeof (struct GUI_Vertex), (void*) (2 * sizeof (float)));
	glEnableVertexAttribArray (1);
}


__attribute__((packed))
struct GUI_RectangleSWNE
{
	struct v2f32_xy sw; //Bottom left
	struct v2f32_xy ne; //Top right
};


__attribute__((packed))
struct GUI_ElementsRectangle
{
	unsigned int v [6];
};


void GUI_RectangleSWNE_SE 
(struct v2f32_xy * se, struct GUI_RectangleSWNE const * r)
{
	se->v [0] = r->ne.v [0];
	se->v [1] = r->sw.v [1];
}


void GUI_RectangleSWNE_NW 
(struct v2f32_xy * nw, struct GUI_RectangleSWNE const * r)
{
	nw->v [0] = r->sw.v [0];
	nw->v [1] = r->ne.v [1];
}


void gui_gen_elements (struct GUI_ElementsRectangle e [], size_t n)
{
	for (size_t i = 0; i < n; ++ i)
	{
		e [i].v [0] = 0 + (i * 4);
		e [i].v [1] = 1 + (i * 4);
		e [i].v [2] = 3 + (i * 4);
		e [i].v [3] = 1 + (i * 4);
		e [i].v [4] = 2 + (i * 4);
		e [i].v [5] = 3 + (i * 4);
	}
}


void gui_gen_vertices 
(struct GUI_VertexRectangle v [], struct GUI_RectangleSWNE const x [], size_t xn)
{
	srand (0);
	for (size_t i = 0; i < xn; ++ i)
	{
		vf32_cpy (v [i].sw.p, x [i].sw.v, 2);
		vf32_random (v [i].sw.c, 3);
		v [i].sw.c [3] = 1.0f;
		
		GUI_RectangleSWNE_NW ((struct v2f32_xy *)v [i].nw.p, x + i);
		vf32_random (v [i].nw.c, 3);
		v [i].nw.c [3] = 1.0f;
		
		vf32_cpy (v [i].ne.p, x [i].ne.v, 2);
		vf32_random (v [i].ne.c, 3);
		v [i].ne.c [3] = 1.0f;
		
		GUI_RectangleSWNE_SE ((struct v2f32_xy *)v [i].se.p, x + i);
		vf32_random (v [i].se.c, 3);
		v [i].se.c [3] = 1.0f;
	}
}


#define GUI_COLOR1 1 << 0

struct GUI_Object
{
	uint32_t flags;
};


struct GUI
{
	GLuint vao;
	GLuint vbo;
	GLuint ebo;
	
	struct GUI_VertexRectangle * vertices;
	struct GUI_RectangleSWNE * rectangles;
	struct GUI_Object * objects;
	size_t n;
};





void gui_init (struct GUI * g, size_t n)
{
	size_t v8 = sizeof (struct GUI_VertexRectangle) * n;
	size_t e8 = sizeof (struct GUI_ElementsRectangle) * n;
	struct GUI_ElementsRectangle * elements;
	g->n = n;
	//calloc allocates and set memory data to zero.
	g->rectangles = calloc (n, sizeof (struct GUI_RectangleSWNE));
	g->vertices = calloc (n, sizeof (struct GUI_VertexRectangle));
	g->objects = calloc (n, sizeof (struct GUI_Object));
	elements = malloc (e8);
	glGenVertexArrays (1, &g->vao);
	glGenBuffers (1, &g->vbo);
	glGenBuffers (1, &g->ebo);
	gui_gen_elements (elements, n);
	gui_vertex_layout (g->vao, g->vbo, g->ebo, v8, e8, NULL, elements);
	free (elements);
}


void gui_sync (struct GUI * g)
{
	gui_gen_vertices (g->vertices, g->rectangles, g->n);
	size_t v8 = sizeof (struct GUI_VertexRectangle) * g->n;
	glBufferSubData (GL_ARRAY_BUFFER, 0, v8, g->vertices);
}


void gui_draw (struct GUI * g, GLint uniform_color)
{
	//Number of rectangles (g->n).
	size_t const n = g->n;
	//Vertex array object.
	GLuint const vao = g->vao;
	
	struct GUI_Object const * o = g->objects;
	
	glBindVertexArray (vao);
	glDrawElements (GL_TRIANGLES, n * 6, GL_UNSIGNED_INT, 0);
	
	for (size_t i = 0; i < n; ++ i)
	{
		if (o [i].flags & GUI_COLOR1)
		{
			glUniform4f (uniform_color, 1.0, 0.0, 0.0, 0.0);
			size_t m = i * 6 * sizeof (unsigned int);
			glDrawElements (GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)m);
		}
	}
	glUniform4f (uniform_color, 0.0, 0.0, 0.0, 0.0);
}


void gui_intersect_point (struct GUI * g, float p [2], int32_t flags_add, int32_t flags_remove)
{
	//Number of rectangles (g->n).
	size_t const n = g->n;
	
	//Rectangles ((SW, NE), (SW, NE)) <=> ((vec2, vec2), (vec2, vec2))
	//struct v2f32_xy * x = (struct v2f32_xy *) g->rectangles;
	struct GUI_RectangleSWNE * r = g->rectangles;
	
	//vf32_print (stdout, p, 2, "%f ");
	
	for (size_t i = 0; i < n; ++ i)
	{
		float d [2];
		v2f32_sub (d, r [i].sw.v, p);
		//vf32_print (stdout, d, 2, "%f ");
		if ((d [0] > 0.0f) || (d [1] > 0.0f)) {continue;}
		v2f32_sub (d, r [i].ne.v, p);
		//vf32_print (stdout, d, 2, "%f ");
		if ((d [0] < 0.0f) || (d [1] < 0.0f)) {continue;}
		g->objects [i].flags |= flags_add;
		g->objects [i].flags &= ~flags_remove;
		//printf ("%x\n", g->objects [i].flags);
	}
}







