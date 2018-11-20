#pragma once

#include "SDLGL.h"
#include "debug.h"


struct ProDraw
{
	uint32_t n;
	uint32_t * offset;
	uint32_t * length;
	uint32_t * capacity;
	GLenum * primitive;
};


void pd_init (struct ProDraw * pd)
{
	pd->offset = calloc (pd->n, sizeof (uint32_t));
	pd->length = calloc (pd->n, sizeof (uint32_t));
	pd->capacity = calloc (pd->n, sizeof (uint32_t));
	pd->primitive = calloc (pd->n, sizeof (GLenum));
}


uint32_t pd_cap (struct ProDraw * pd)
{
	if (pd->n == 0) {return 0;}
	uint32_t last = pd->n - 1;
	return pd->offset [last] + pd->capacity [last];
}


