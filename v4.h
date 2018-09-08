#pragma once

#include "v.h"


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


void v4f32_normalize (float r [4], float const a [4])
{
	vf32_normalize (r, a, 4);
}


float v4f32_norm2 (float const a [4])
{
	return vf32_dot (a, a, 4);
}
