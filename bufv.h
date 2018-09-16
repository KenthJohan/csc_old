#pragma once




#define BUFV(type, name) \
{ \
	type * p; \
	size_t n; \
	size_t i; \
} name

#define BUFV_INIT(type, buf, count) \
buf.n = count; \
buf.i = 0; \
buf.p = (type *) calloc (buf.n, sizeof (type))

#define BUFV_PUSH(buf, count) \
(buf.p + buf.i); \
/*ASSERT ((buf.i + count) < buf.n);*/ \
buf.i += count

#define BUFV_FULL(buf) ((buf.i+1) >= buf.n)


#define BUFV_LOOP(buf, type, e) for (type (e) = buf.p; (e) < (buf.p + buf.n); ++ e)
