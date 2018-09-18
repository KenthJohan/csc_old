#pragma once

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>


#define BUFV(type, name) \
{ \
	type * p; \
	size_t c; \
	size_t n; \
} name

#define BUFV_INIT(type, buf, cap) \
buf.c = cap; \
buf.n = 0; \
buf.p = (type *) calloc (buf.c, sizeof (type))

#define BUFV_PUSH(buf, count, x) \
if ((buf.n + count - 1) < buf.c) {x = buf.p + buf.n; buf.n ++;} \
else {x = NULL;}
/*ASSERT ((buf.i + count) < buf.n);*/

#define BUFV_FULL(buf) (buf.n >= buf.c)

/*
#define BUFV_FREE(buf) \
free (buf.p); \
buf.p = NULL; \
buf.c = 0; \
buf.n = 0
*/

#define BUFV_LOOP(buf, type, e) for (type (e) = buf.p; (e) < (buf.p + buf.n); ++ e)
