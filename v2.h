#pragma once


struct v2f32_xy
{
	float x;
	float y;
};


// ret a > b
int v2f32_gt_all (float const * a, float const * b)
{
	return vf32_gt_all (a, b, 2);
}


// ret a < b
int v2f32_lt_all (float const * a, float const * b)
{
	return vf32_lt_all (a, b, 2);
}


// r := a > b
void v2f32_gt (float * r, float const * a, float const * b)
{
	vf32_gt (r, a, b, 2);
}


// r := a < b
void v2f32_lt (float * r, float const * a, float const * b)
{
	vf32_lt (r, a, b, 2);
}



