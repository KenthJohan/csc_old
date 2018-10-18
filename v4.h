#pragma once

#include "v.h"

struct v4f32_XYZW
{
	float x;
	float y;
	float z;
	float w;
};

struct v4f32_XYZW v4f32_XYZW (float x, float y, float z, float w)
{
	struct v4f32_XYZW v;
	v.x = x;
	v.y = y;
	v.z = z;
	v.w = w;
	return v;
}




struct v4f32_rgba
{
	float r;
	float g;
	float b;
	float a;
};


struct v4f32_xywh
{
	float x;
	float y;
	float w;
	float h;
};


//static_assert (sizeof (struct v4f32_rgba) == sizeof (float) * 4, "struct v4f32_rgba is not 4 x float size");
//static_assert (sizeof (struct v4f32_xywh) == sizeof (float) * 4, "struct v4f32_xywh is not 4 x float size");







void v4f32_cpy (float r [4], float const a [4])
{
	vf32_cpy (r, a, 4);
}

void v4f32_add (float r [4], float const a [4], float const b [4])
{
	vf32_add (r, a, b, 4);
}


void v4f32_sub (float r [4], float const a [4], float const b [4])
{
	vf32_sub (r, a, b, 4);
}


void v4f32_set (float r [4], float const b)
{
	vf32_set_scalar (r, b, 4);
}


void v4f32_mul_scalar (float r [4], float const a [4], float const b)
{
	vf32_mul_scalar (r, a, b, 4);
}


void v4f32_mus (float r [4], float const a [4], float const b)
{
	vf32_mul_scalar (r, a, b, 4);
}


void v4f32_normalize (float r [4], float const a [4])
{
	vf32_normalize (r, a, 4);
}


float v4f32_norm2 (float const a [4])
{
	return vf32_dot (a, a, 4);
}
