#pragma once
#include <stdint.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#define STR_MAX(a,b) ((a)>(b)?(a):(b))
#define STR_MIN(a,b) ((a)<(b)?(a):(b))
#define STR_SET_0Z "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"
#define STR_SET_Z0 "ZYXWVUTSRQPONMLKJIHGFEDCBA9876543210"
#define STR_IN(x, a, b) ((a) <= (x) && (x) <= (b))
#define STR_INB(x, a, b, base) STR_IN ((x), (a), STR_MIN ((base) + (a), (b)))
#define STR_UNSIGNED 0
#define STR_SIGNED 1

uint32_t str_to_u32 (char const ** f, int base)
{
	uint32_t a = 0;
	int c;
	while (1)
	{
		c = (**f);
		if (c == '\0') {break;}
		else if (STR_INB (c, '0', '9', abs(base))) {c -= '0';}
		else if (abs(base) > 10 && STR_INB (c, 'a', 'z', abs(base))) {c -= ('a' - 10);}
		else if (abs(base) > 10 && STR_INB (c, 'A', 'Z', abs(base))) {c -= ('A' - 10);}
		else {break;}
		a *= (uint32_t) base;
		a += (uint32_t) c;
		(*f) ++;
	}
	return a;
}

int32_t str_to_i32 (char const ** f, int base)
{
	int32_t a = 0;
	int c;
	int neg = 0;
	c = (**f);
	if (c == '-') {neg = 1; (*f) ++;}
	else if (c == '+') {neg = 0; (*f) ++;}
	while (1)
	{
		c = (**f);
		if (c == '\0') {break;}
		else if (STR_INB (c, '0', '9', abs(base))) {c -= '0';}
		else if (abs(base) > 10 && STR_INB (c, 'a', 'z', abs(base))) {c -= ('a' - 10);}
		else if (abs(base) > 10 && STR_INB (c, 'A', 'Z', abs(base))) {c -= ('A' - 10);}
		else {break;};
		a *= (int32_t) base;
		a += (int32_t) c;
		(*f) ++;
	}
	if (neg) {return -a;}
	else {return a;}
}


int str_from_imax (char * o, uint32_t n, intmax_t value, int base, char pad)
{
	int rem;
	assert (base < (int)sizeof (STR_SET_0Z));
	o += n - 1;
	while (1)
	{
		if (n == 0) {return 0;}
		rem = value % base;
		value /= base;
		if (rem < 0)
		{
			rem += abs (base);
			value += 1;
		}
		*o = STR_SET_0Z [rem];
		n --;
		o --;
		if (value == 0) {break;}
	}
	while (1)
	{
		if (n == 0) {return 0;}
		*o = pad;
		n --;
		o --;
	}
	return 0;
}



void strf (char * o, uint32_t n, char const * f, ...)
{
	va_list va;
	va_start (va, f);
	uint32_t size = 0;
	int base = 10;
	union
	{
		uint8_t au8;
		uint16_t au16;
		uint32_t au32;
		uint64_t au64;
	} arg;
	while (1)
	{
		switch (*f)
		{
		case '\0': goto end;
		case '%': f ++; break;
		default: *o = *f; o ++; n --; f ++; continue;
		}

		switch (*f)
		{
		case 'u': f ++; size = str_to_u32 (&f, 10); break;
		}

		switch (size)
		{
		case 8: arg.au8 = va_arg (va, uint8_t); break;
		case 16: arg.au16 = va_arg (va, uint16_t); break;
		case 32: arg.au32 = va_arg (va, uint32_t); break;
		case 64: arg.au64 = va_arg (va, uint64_t); break;
		}

		switch (*f)
		{
		case '_': f ++; base = (int)str_to_i32 (&f, 10); break;
		}

		str_from_imax (o, n, arg.au32, base, ' ');
		//snprintf (o, n, "%u", arg.au32);
	}
	va_end (va);
end:
	return;
}
