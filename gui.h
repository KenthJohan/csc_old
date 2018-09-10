#pragma once

#include "v.h"
#include "v2.h"


__attribute__((packed))
struct GUI_Vertex
{
	float p [2];
	float c [4];
};


__attribute__((packed))
struct GUI_RectangleSWNE
{
	struct v2f32_xy sw; //Bottom left
	struct v2f32_xy ne; //Top right
};


__attribute__((packed))
struct GUI_RectangleElements
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


void gui_create_elements_from_rectangle (struct GUI_RectangleElements e [], size_t n)
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


void gui_create_vertex_from_rectangle 
(struct GUI_Vertex v [], size_t vn, struct GUI_RectangleSWNE const x [], size_t xn)
{
	srand (0);
	size_t j = 0;
	for (size_t i = 0; i < xn; ++ i)
	{
		vf32_cpy (v [j].p, x [i].sw.v, 2);
		vf32_random (v [j].c, 3);
		v [j].c [3] = 1.0f;
		j ++;
		
		GUI_RectangleSWNE_NW ((struct v2f32_xy *)v [j].p, x + i);
		vf32_random (v [j].c, 3);
		v [j].c [3] = 1.0f;
		j ++;
		
		vf32_cpy (v [j].p, x [i].ne.v, 2);
		vf32_random (v [j].c, 3);
		v [j].c [3] = 1.0f;
		j ++;
		
		GUI_RectangleSWNE_SE ((struct v2f32_xy *)v [j].p, x + i);
		vf32_random (v [j].c, 3);
		v [j].c [3] = 1.0f;
		j ++;
	}
}




void gui_hit (SDL_Window * window, SDL_MouseButtonEvent * m, struct GUI_RectangleSWNE const r [], size_t n)
{
	float x [2];
	ui_mouse (x, window, m);
	size_t hn = 2*2*2;
	float h [2*2*2];
	vf32_sub2 (h, (float const *)r, x, 4, 2);
	vf32_print (stdout, x, 2, "%f ");
	vf32_print2 (stdout, h, 4, 2, "%f ");
}




