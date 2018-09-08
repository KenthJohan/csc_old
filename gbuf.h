#pragma once

#include <stdint.h>
#include <stdlib.h>
#include "debug.h"

#define GBUF_MALLOC 1 << 0
//#define GBUF_RING 1 << 1

/*
#define GBUF_UNKNOWN 0
#define GBUF_F32 1
#define GBUF_F64 2
#define GBUF_S8 3
#define GBUF_S16 4
#define GBUF_S32 5
#define GBUF_S64 6
#define GBUF_S128 7
#define GBUF_U8 3
#define GBUF_U16 4
#define GBUF_U32 5
#define GBUF_U64 6
#define GBUF_U128 7
*/

#define GBUF_LOOP(t, i, d) for (t (i) = 0; i < gbuf_count ((d)); (i) = (i) + 1)
#define GBUF_LOOP_CAPACITY(t, i, d) for (t (i) = 0; i < gbuf_capacity ((d)); (i) = (i) + 1)

struct GBuf
{
	uint16_t flags;
	
	//Element size as 8 bit unit
	uint16_t element_size8;
	
	//Element count
	uint32_t count;
	
	//Capacity count
	uint32_t capacity;
};



void * gbuf_init (void * data, uint32_t capacity, uint8_t element_size8, uint8_t flags)
{
	TRACE_F ("element_size8 %i", element_size8);
	struct GBuf * g = data;
	if ((g == NULL) && (flags & GBUF_MALLOC))
	{
		size_t n = capacity * element_size8 + sizeof (struct GBuf);
		g = malloc (n);
		ASSERT (g != NULL);
		//memset (g, 0, n);
	}
	ASSERT (g != NULL);
	g->count = 0;
	g->capacity = capacity;
	g->element_size8 = element_size8;
	g->flags = flags;
	return g + 1;
}


struct GBuf * gbuf_meta (void * data)
{
	struct GBuf * g = data;
	g = g - 1;
	return g;
}

uint32_t gbuf_count (void * data)
{
	struct GBuf * g = gbuf_meta (data);
	return g->count;
}


uint32_t gbuf_capacity (void * data)
{
	struct GBuf * g = gbuf_meta (data);
	return g->capacity;
}

uint32_t gbuf_cap8 (void * data)
{
	struct GBuf * g = gbuf_meta (data);
	return g->capacity * g->element_size8;
}



void * gbuf_add (void * data)
{
	struct GBuf * g = gbuf_meta (data);
	if (g->count >= g->capacity) {return NULL;}
	g->count ++;
	uintptr_t d = (uintptr_t)data + g->element_size8 * (g->count - 1);
	return (void *)d;
}


void * gbuf_get (void * data, size_t index)
{
	struct GBuf * g = gbuf_meta (data);
	if (index >= g->count) {return NULL;}
	uintptr_t d = (uintptr_t)data + g->element_size8 * index;
	return (void *)d;
}


/*
void gbuf_print (struct GBuf * g)
{
	for (size_t i = 0; i < n; i ++)
	{
		printf ("%f %f\n", x [i].x, x [i].y);
	}
}
* */




