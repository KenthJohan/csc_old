#pragma once
#include <math.h>

// r := a . b
float vf32_dot (float const * a, float const * b, size_t n)
{
	float r = 0;
	for (size_t i = 0; i < n; ++ i)
	{
		r += a [i] * b [i];
	}
	return r;
}


// r := a + b
void vf32_add (float * r, float const * a, float const * b, size_t n)
{
	for (size_t i = 0; i < n; ++ i)
	{
		r [i] = a [i] + b [i];
	}
}


// r := a - b
void vf32_sub (float * r, float const * a, float const * b, size_t n)
{
	for (size_t i = 0; i < n; ++ i)
	{
		r [i] = a [i] - b [i];
	}
}


// r := a < b
void vf32_lt (float * r, float const * a, float const * b, size_t n)
{
	for (size_t i = 0; i < n; ++ i)
	{
		r [i] = a [i] < b [i];
	}
}

// r := a > b
void vf32_gt (float * r, float const * a, float const * b, size_t n)
{
	for (size_t i = 0; i < n; ++ i)
	{
		r [i] = a [i] > b [i];
	}
}


// ret a > b
int vf32_gt_all (float const * a, float const * b, size_t n)
{
	for (size_t i = 0; i < n; ++ i)
	{
		if (a [i] <= b [i]) {return 0;}
	}
	return 1;
}


// ret a < b
int vf32_lt_all (float const * a, float const * b, size_t n)
{
	for (size_t i = 0; i < n; ++ i)
	{
		if (a [i] <= b [i]) {return 0;}
	}
	return 1;
}


// Set all element (x) of r to b
// r := {x | x = b}
void vf32_set_scalar (float * r, float const b, size_t n)
{
	for (size_t i = 0; i < n; ++ i)
	{
		r [i] = b;
	}
}

// r := a * b
void vf32_mul_scalar (float * r, float const * a, float const b, size_t n)
{
	for (size_t i = 0; i < n; ++ i)
	{
		r [i] = a [i] * b;
	}
}


// r := a
void vf32_cpy (float * r, float const * a, size_t n)
{
	for (size_t i = 0; i < n; ++ i)
	{
		r [i] = a [i];
	}
}


// ret |a|^2
float vf32_norm2 (float const * a, size_t n)
{
	return vf32_dot (a, a, n);
}


// ret |a|
float vf32_norm (float const * a, size_t n)
{
	return sqrtf (vf32_norm2 (a, n));
}


// r := a / |a|
void vf32_normalize (float * r, float const * a, size_t n)
{
	float const l = vf32_norm (a, n);
	float const s = l > 0.0f ? 1.0f / l : 0.0f;
	vf32_mul_scalar (r, a, s, n);
}












int vf32_ray_sphere_intersect 
(float p [3], float d [3], float sc [3], float sr, float *t, float q [3]) 
{
	//Vector m = p - s.c;
	float m [3];
	vf32_sub (p, sc, m, 3);
	//float b = Dot(m, d); 
	float b = vf32_dot (m, d, 3);
	//float c = Dot(m, m) - s.r * s.r; 
	float c = vf32_dot (m, m, 3) - (sr * sr);

	// Exit if r’s origin outside s (c > 0) and r pointing away from s (b > 0) 
	if (c > 0.0f && b > 0.0f) {return 0;}
	float discr = (b * b) - c; 

	// A negative discriminant corresponds to ray missing sphere 
	if (discr < 0.0f) {return 0;}

	// Ray now found to intersect sphere, compute smallest t value of intersection
	*t = -b - sqrtf (discr); 

	// If t is negative, ray started inside sphere so clamp t to zero 
	if (*t < 0.0f) {*t = 0.0f;}
	*t = MAX (*t, 0.0f);
	
	//q = p + dt;
	vf32_mul_scalar (q, d, *t, 3);
	vf32_add (q, p, q, 3);
	
	return 1;
}










void vf32_print (FILE * f, float const x [], size_t n, char const * format)
{
	fprintf (f, "(");
	for (size_t i = 0; i < n; ++ i)
	{
		fprintf (f, format, x [i]);
	}
	fprintf (f, "\b)\n");
	fflush (f);
}








void vf32_print2 (FILE * f, float const x [], size_t n1, size_t n2, char const * format)
{
	for (size_t i = 0; i < n1; ++ i)
	{
		vf32_print (f, x + (i*n2), n2, format);
	}
}





















// r := a - b
void vf32_sub2 (float * r, float const * a, float const * b, size_t an, size_t bn)
{
	for (size_t i = 0; i < an; ++ i)
	{
		vf32_sub (r + (i*bn), a + (i*bn), b, bn);
	}
}



