#pragma once

#include "v.h"

struct v3f32_XYZ
{
	float x;
	float y;
	float z;
};

struct v3f32_XYZ v3f32_XYZ (float x, float y, float z)
{
	struct v3f32_XYZ v;
	v.x = x;
	v.y = y;
	v.z = z;
	return v;
}


void v3f32_cpy (float r [3], float const a [3])
{
	vf32_cpy (3, r, a);
}


void v3f32_add (float r [3], float const a [3], float const b [3])
{
	vf32_add (3, r, a, b);
}


void v3f32_mul (float r [3], float const a [3], float const b [3])
{
	vf32_mul (3, r, a, b);
}


void v3f32_sub (float r [3], float const a [3], float const b [3])
{
	vf32_sub (3, r, a, b);
}


void v3f32_set (float r [3], float const b)
{
	//vf32_set_scalar (3, r, b);
}


void v3f32_mul_scalar (float r [3], float const a [3], float const b)
{
	//vf32_mul_scalar (r, a, b, 3);
}


void v3f32_mus (float r [3], float const a [3], float const b)
{
	//vf32_mul_scalar (r, a, b, 3);
}


void v3f32_normalize (float r [3], float const a [3])
{
	vf32_normalize (3, r, a);
}


float v3f32_dot (float const a [3], float const b [3])
{
	return vf32_dot (3, a, b);
}


float v3f32_norm2 (float const a [3])
{
	return vf32_dot (3, a, a);
}


float v3f32_dist2 (float const a [4], float const b [4])
{
	float d [3];
	v3f32_sub (d, a, b);
	return v3f32_norm2 (d);
}





void v3u8_set3 (uint8_t r [3], uint8_t const x0, uint8_t const x1, uint8_t const x2)
{
	r [0] = x0;
	r [1] = x1;
	r [2] = x2;
}
