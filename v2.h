#pragma once


struct v2f32_xy
{
	float v [2];
};


// ret a > b
int v2f32_gt_all (float const a [2], float const b [2])
{
	return vf32_gt_all (a, b, 2);
}


// ret a < b
int v2f32_lt_all (float const a [2], float const b [2])
{
	return vf32_lt_all (a, b, 2);
}


// r := a > b
void v2f32_gt (float r [2], float const a [2], float const b [2])
{
	vf32_gt (r, a, b, 2);
}


// r := a < b
void v2f32_lt (float r [2], float const a [2], float const b [2])
{
	vf32_lt (r, a, b, 2);
}


// r := a - b
void v2f32_sub (float r [2], float const a [2], float const b [2])
{
	vf32_sub (r, a, b, 2);
}


float v2f32_dot (float const a [2], float const b [2])
{
	return vf32_dot (a, b, 2);
}


float v2f32_norm2 (float const a [2])
{
	return vf32_dot (a, a, 2);
}


float v2f32_dist2 (float const a [2], float const b [2])
{
	float d [2];
	v2f32_sub (d, a, b);
	return v2f32_norm2 (d);
}
