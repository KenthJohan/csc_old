#pragma once

#include "debug.h"
#include "global.h"
#include "m.h"
#include "v.h"

float dot (float const * a, float const * b, size_t n)
{
	float r = 0;
	for (int i = 0; i < n; i = i + 1)
	{
		r += a [i] * b [i];
	}
	return r;
}


void sub (float const * a, float const * b, float * r, size_t n)
{
	for (int i = 0; i < n; i = i + 1)
	{
		r [i] = a [i] - b [i];
	}
}


void mul (float const * a, float const * b, float * r, size_t n)
{
	for (int i = 0; i < n; i = i + 1)
	{
		r [i] = a [i] * b [i];
	}
}


float length2 (float const * a, float const * b, float const * s, float * d, size_t n)
{
	float d2;
	sub (a, b, d, n);
	mul (d, s, d, n);
	d2 = dot (d, d, n);
	return d2;
}



#define POI_T 0
#define POI_X 1
#define POI_Y 2

#define POI_REMOVED 1 << 0
#define POI_MERGED 1 << 1


struct POI;

struct POI
{
	uint32_t camera;
	float r2_txy_max; //Radius^2 of (t,x,y)
	float r2_txy; //Radius^2 of (t,x,y)
	float r2_xy; //Raduis^2 of (0,x,y)
	uint32_t l; //Length of the path.
	struct POI * next; //Linked list
	struct POI * prev; //Linked list
	uint32_t g; //Group number
	float p [3]; //(t, x, y)
	uint32_t flags;
};


int poi_read (FILE * f, struct POI * x)
{
	int r;
	char c;
	r = fscanf (f, "%f%f%f%c", x->p + POI_T, x->p + POI_X, x->p + POI_Y, &c);
	return r;
}



int poi_load (FILE * f, struct POI * y, size_t n)
{
	//TRACE_F ("%i\n", n);
	int i;
	int r;
	for (i = 0; i < n; i = i + 1)
	{
		if (n == 0) {return i;};
		char c;
		r = fscanf (f, "%f%f%f%c", y->p + POI_T, y->p + POI_X, y->p + POI_Y, &c);
		//TRACE_F ("%i %i %i %f", i, n, r, y->p [0]);
		if (r == EOF) {return i;}
		y = y + 1;
	}
	return i;
}


int poi_save (FILE * f, struct POI * y, size_t n)
{
	int r;
	for (size_t i = 0; i < n; i = i + 1)
	{
		r = fprintf (f, "%e %e %e\n", y->p [POI_T], y->p [POI_X], y->p [POI_Y]);
		y = y + 1;
		//TRACE_F ("%i", n);
	}
	return r;
}


int poi_load_fromfile (char const * name, struct POI * y, size_t n)
{
	FILE * f = fopen (name, "r");
	int r = poi_load (f, y, n);
	fclose (f);
	return r;
}


int poi_save_tofile (char const * name, struct POI * y, size_t n)
{
	FILE * f = fopen (name, "w");
	int r = poi_save (f, y, n);
	fclose (f);
	return r;
}



void poi_reset (struct POI * x0, size_t n)
{
	LOOPN (size_t, i, n)
	{
		struct POI * x = x0 + i;
		x->l = 0;
		x->g = 0;
		x->r2_txy = INT32_MAX;
		x->r2_txy_max = 400 * 400;
		x->r2_xy = INT32_MAX;
		x->next = NULL;
		x->prev = NULL;
		x->flags = 0;
	}
}


struct POI_Context
{
	uint32_t g; //Group number
	float s [3];
};


void dev_search6_merge (struct POI_Context * ctx, struct POI * x0, size_t n)
{
	LOOPP (size_t, ai, bi, n)
	{
		struct POI * a = x0 + ai;
		struct POI * b = x0 + bi;
		if (a->flags & POI_REMOVED) {continue;} //Ignore removed points
		if (a->p [POI_T] != b->p [POI_T]) {continue;} //Only compare same frame
		float d [2];
		float r2_xy = length2 (a->p + 1, b->p + 1, ctx->s + 1, d, COUNTOF (d));
		if (r2_xy > 500) {continue;} //Ignore outside search radius
		//Move a and b to avarage pos. Set remove flag on b.
		//vf32_add (a->p + 1, b->p + 1, a->p + 1, 2);
		//vf32_mul_scalar (a->p + 1, a->p + 1, 0.5f, 2);
		
		//MUL_VVS (2, a->p + 1, a->p + 1, 0.5f);
		//SET_VV (2, b->p + 1, a->p + 1);
		a->flags |= POI_MERGED;
		b->flags |= POI_REMOVED;
	}
}


void dev_search1_distance (struct POI_Context * ctx, struct POI * x0, size_t n)
{
	LOOPP (size_t, ai, bi, n)
	{
		struct POI * a = x0 + ai;
		struct POI * b = x0 + bi;
		if (a->flags & POI_REMOVED) {continue;}
		if (b->flags & POI_REMOVED) {continue;}
		if (a->p [POI_T] > b->p [POI_T]) {continue;} //Only go forward in time
		if ((a->p [POI_T] + 4) < b->p [POI_T]) {continue;} //Time limit
		float d [3];
		float r2_txy = length2 (a->p, b->p, ctx->s, d, COUNTOF (d));
		if (r2_txy > a->r2_txy_max) {continue;}
		if (r2_txy < a->r2_txy)
		{
			a->r2_xy = length2 (a->p + 1, b->p + 1, ctx->s + 1, d, 2);
			a->r2_txy = r2_txy;
			a->next = b;
		}
	}
}





void dev_search2_length (struct POI_Context * ctx, struct POI * x0, size_t n)
{
	LOOPN (size_t, i, n)
	{
		struct POI * a = x0 + i;
		struct POI * b = a;
		int l = 0;
		if (b->l != 0) {continue;}
		while (1)
		{
			if (b == NULL) {break;}
			l ++;
			b = b->next;
		}
		b = a;
		while (1)
		{
			if (b == NULL) {break;}
			b->l = l;
			b = b->next;
		}
	}
}


void dev_search3_backtrack (struct POI_Context * ctx, struct POI * x0, size_t n)
{
	LOOPN (size_t, i, n)
	{
		struct POI * x = x0 + i;
		if (x->next == NULL) {continue;}
		x->next->prev = x;
	}
}



void dev_search4_relink (struct POI_Context * ctx, struct POI * x0, size_t n)
{
	LOOPN (size_t, i, n)
	{
		struct POI * a = x0 + i;
		if (a->next == NULL) {continue;}
		struct POI * b = a->next->prev;
		if (b == NULL) {continue;}
		if (b == a) {continue;}
		if (a->r2_txy < b->r2_txy) 
		{
			a->next->prev = a;
			b->next = NULL;
		}
		else
		{
			b->next->prev = b;
			a->next = NULL;
		}

		//TRACE_F ("%s", "");
	}
}




void dev_search5_group (struct POI_Context * ctx, struct POI * x0, size_t n)
{
	LOOPN (size_t, i, n)
	{
		struct POI * a = x0 + i;
		if (a->flags & POI_REMOVED) {continue;}
		if (a->prev != NULL) {continue;}
		ctx->g ++;
		//if (x->l < 2) {continue;}
		struct POI * b = a;
		while (1)
		{
			if (b == NULL) {break;}
			if (b->next == b) {break;}
			b->g = ctx->g;
			b = b->next;
		}
	}
}




void dev_search8_filter (struct POI_Context * ctx, struct POI * x0, size_t n)
{
	LOOPN (size_t, i, n)
	{
		struct POI * a = x0 + i;
		if (a->l < 10)
		{
			a->flags |= POI_REMOVED;
		}
		
		//if (a->p [POI_X] < 400.0f) {a->g = 0;}
		//if (a->p [POI_X] > (3840-400)) {a->g = 0;}
		if (a->p [POI_Y] > (2160-100)) 
		{
			a->flags |= POI_REMOVED;
			//a->g = 0;
		}
		//
		//TRACE_F ("remove %i %f", a->l, a->p [POI_Y]);
	}
}


uint32_t dev_num_pos_g (struct POI * x0, size_t n)
{
	uint32_t sum = 0;
	LOOPN (size_t, i, n)
	{
		struct POI * a = x0 + i;
		if (a->g > 0)
		{
			sum ++;
		}
	}
	return sum;
}





void dev_intersect (struct POI * x0, size_t n, float p0 [3], float p1 [3])
{
	float d [3];
	vf32_sub (p0, p1, d, 3);
	vf32_normalize (d, d, 3);
	for (size_t i = 0; i < n; ++ i)
	{
		//Sphere center
		float * c = x0 [i].p;
		//Sphere raduis
		float r = 0.5f;
		//Outputs
		float q [4];
		float t;
		int res = vf32_ray_sphere_intersect (p0, d, c, r, &t, q);
		if (res > 0)
		printf ("%i %i\n", (int)i, res);
	}
}





