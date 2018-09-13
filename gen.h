#pragma once

#include "map.h"
#include "v.h"
#include "c.h"


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



void gen_economic_curve (uint8_t * o, size_t w, size_t h, size_t d)
{
	memset (o, 0, w * h * d);
	float * f = malloc (w);
	for (size_t i = 0; i < 100; ++ i)
	{
		f [i] = cos ((float)i / 40.0f);
	}
	
	for (size_t ix = 0; ix < w; ++ ix)
	{
		size_t iy = f [ix];
		size_t const i = (d * w * iy) + (d * ix);
		o [i+0] = 1.0f;
		o [i+1] = 1.0f;
		o [i+2] = 1.0f;
		o [i+3] = 1.0f;
	}
	free (f);
}




