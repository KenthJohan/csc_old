#pragma once

#include "vertex.h"
#include "gbuf.h"
#include "map.h"
#include "v.h"
#include "c.h"


struct Grid_Layout
{
	float r;
};

void gen1_grid 
(
	struct Vertex * v0,
	float z, 
	float x1, 
	float x2, 
	float y1, 
	float y2, 
	float r,
	float cc [4]
)
{
	float x = x1;
	float y = y1;
	while (1)
	{
		struct Vertex * v;
		float * p;
		float * c;
		v = gbuf_add (v0);
		p = v->pos;
		c = v->col;
		if (v == NULL) {break;}
		p [0] = x;
		p [1] = z;
		p [2] = y1;
		p [3] = 1.0f;
		memcpy (c, cc, 4 * sizeof (float));

		v = gbuf_add (v0);
		p = v->pos;
		c = v->col;
		if (v == NULL) {break;}
		p [0] = x;
		p [1] = z;
		p [2] = y2;
		p [3] = 1.0f;
		memcpy (c, cc, 4 * sizeof (float));
		
		v = gbuf_add (v0);
		p = v->pos;
		c = v->col;
		if (v == NULL) {break;}
		p [0] = x1;
		p [1] = z;
		p [2] = y;
		p [3] = 1.0f;
		memcpy (c, cc, 4 * sizeof (float));
		
		v = gbuf_add (v0);
		p = v->pos;
		c = v->col;
		if (v == NULL) {break;}
		p [0] = x2;
		p [1] = z;
		p [2] = y;
		p [3] = 1.0f;
		memcpy (c, cc, 4 * sizeof (float));
		
		x += r;
		y += r;
	}
}



void gen1_circle (struct Vertex * v0, size_t n, float r, float x, float y, float z)
{
	for (size_t i = 0; i < n; i = i + 1)
	{
		struct Vertex * v = gbuf_add (v0);
		//V4_SET (v [i].col, 0.0f, 0.0f, 1.0f, 1.0f);
		float a = (2 * M_PI) * ((float)i / (float)n);
		v->pos [0] = x + r * cos (a);
		v->pos [1] = y + r * sin (a);
		v->pos [2] = z;
		v->pos [3] = 1.0f;
		v->col [0] = 1.0f;
		v->col [1] = 1.0f;
		v->col [2] = 1.0f;
		v->col [3] = 1.0f;
		//printf ("%f %f %f %f : %f %f %f %f\n", v->pos [0], v->pos [1], v->pos [2], v->pos [3], v->col [0], v->col [1], v->col [2], v->col [3]);
	}
}



void gen_circle (struct Vertex * v, size_t n, float r, float x, float y, float z)
{
	for (size_t i = 0; i < n; i = i + 1)
	{
		//V4_SET (v [i].col, 0.0f, 0.0f, 1.0f, 1.0f);
		float a = (2 * M_PI) * ((float)i / (float)n);
		v [i].pos [0] = x + r * cos (a);
		v [i].pos [1] = y + r * sin (a);
		v [i].pos [2] = z;
		v [i].pos [3] = 1.0f;
		//printf ("%f %f %f %f : %f %f %f %f\n", v [i].pos [0], v [i].pos [1], v [i].pos [2], v [i].pos [3], v [i].col [0], v [i].col [1], v [i].col [2], v [i].col [3]);
	}
}


void gen_circle1 (struct Vertex * v, size_t n, float r, float x, float y, float z)
{
	size_t i = 0;
	float a;
	if (i >= n) {return;}
	//V4_SET (v [i].col, 0.0f, 0.0f, 1.0f, 1.0f);
	v [i].pos [0] = x;
	v [i].pos [1] = y;
	v [i].pos [2] = z;
	v [i].pos [3] = 1.0f;
	i ++;
	
	while (1)
	{
		if (i >= n) {break;}
		a = (2 * M_PI) * ((float)i / (float)(n));
		v [i].pos [0] = x + r * cos (a);
		v [i].pos [1] = y + r * sin (a);
		v [i].pos [2] = z;
		v [i].pos [3] = 1.0f;
		i ++;
	}
}




void gen_pixmap_sample (uint8_t * data, size_t w, size_t h, size_t d)
{
	for (size_t y = 0; y < h; ++ y)
	for (size_t x = 0; x < w; ++ x)
	//for (size_t c = 0; c < d; ++ c)
	{
		//size_t i = d*w*y + d*x + c;
		size_t i = d*w*y + d*x;
		uint8_t c = x*y;
		data [i + 0] = c;
		data [i + 1] = c;
		data [i + 2] = c;
		data [i + 3] = 255;
	}
}


void gen_mandelbrot_pallete (uint8_t * o, size_t count, size_t depth)
{
	ASSERT (depth == 4);
	float s [3] = {0};
	s [0] = 360.0f / (float)(depth * count);
	//s [1] = 1.0f / (float)(depth * count);
	s [2] = 0.5f / (float)(depth * count);
	float t [3] = {0};
	t [0] = 0.0f;
	t [1] = 1.0f;
	t [2] = 0.5f;

	for (size_t i = 0; i < (depth * count); i += depth)
	{
		float hsv [3];
		float rgb [3];
		hsv [0] = (float)i * s [0] + t [0];
		hsv [1] = (float)i * s [1] + t [1];
		hsv [2] = (float)i * s [2] + t [2];
		rgb_from_hsv (rgb, hsv);
		//TRACE_F ("%f %f %f %f", hsv [0], rgb [0], rgb [1], rgb [2]);
		o [i + 0] = rgb [0] * 255;
		o [i + 1] = rgb [1] * 255;
		o [i + 2] = rgb [2] * 255;
		o [i + 3] = 255;
	}
}

void gen_mandelbrot_pixmap 
(uint8_t * o, uint8_t const * pallete, size_t w, size_t h, size_t d, size_t max_iteration)
{
	ASSERT (d == 4);
	//For each pixel (ix, iy) on the screen, do:
	for (size_t ix = 0; ix < w; ++ ix)
	for (size_t iy = 0; iy < h; ++ iy)
	{
		size_t const i = (d * w * iy) + (d * ix);
		//Scaled x coordinate of pixel (scaled to lie in the Mandelbrot X scale (-2.5, 1))
		//Scaled y coordinate of pixel (scaled to lie in the Mandelbrot Y scale (-1, 1))
		float x0 = map_lin_float (ix, 0.0f, (float) w, -2.5f, 1.0f);
		float y0 = map_lin_float (iy, 0.0f, (float) h, -1.0f, 1.0f);
		float x = 0.0f;
		float y = 0.0f;
		size_t j;
		for (j = 0; j < max_iteration-1; ++ j)
		{
			if (x*x + y*y >= 2*2) {break;}
			float xt = (x * x) - (y * y) + x0;
			y = (2.0f * x * y) + y0;
			x = xt;
		}
		ASSERT (j < d*max_iteration);
		memcpy (o + i, pallete + (j*d), d);
		//TRACE_F ("%i", i);
	}
}






