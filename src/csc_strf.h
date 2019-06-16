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

#define STR_SIGNED (1 << 0)
#define STR_UNSIGNED (1 << 1)

uint32_t str_to_u32 (char const ** f, int8_t base)
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

int32_t str_to_i32 (char const ** f, int8_t base)
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

int8_t str_to_i8 (char const ** f, int8_t base)
{
	int32_t v = str_to_i32 (f, base);
	return (int8_t) v;
}

uint8_t str_to_u8 (char const ** f, int8_t base)
{
	uint32_t v = str_to_u32 (f, base);
	return (uint8_t) v;
}


void str_from_imax (char * o, uint32_t n, intmax_t value, int8_t base, char pad)
{
	assert (base != 0);
	assert (o != NULL);
	int rem;
	int negative = 0;
	assert (base < (int8_t)sizeof (STR_SET_0Z));
	o += n - 1;
	if (value < 0)
	{
		negative = 1;
		value = -value;
	}
	while (1)
	{
		if (n == 0) {return;}
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
	if (n > 0 && negative)
	{
		*o = '-';
		n --;
		o --;
	}
	while (1)
	{
		if (n == 0) {return;}
		*o = pad;
		n --;
		o --;
	}
	return;
}



void strf (char * o, uint32_t n, char const * f, ...)
{
	va_list va;
	va_start (va, f);
	uint32_t flag = 0;
	uint32_t size = 0;
	uint32_t width = 0;
	int32_t base = 0;
	intmax_t value = 0;
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
		case 'u':
			f ++;
			flag |= STR_UNSIGNED;
			size = str_to_u32 (&f, 10);
			break;
		case 'i':
			f ++;
			flag |= STR_SIGNED;
			size = str_to_u32 (&f, 10);
			break;
		}

		assert (sizeof (uint8_t) <= sizeof (unsigned));
		assert (sizeof (uint16_t) <= sizeof (unsigned));
		assert (sizeof (int8_t) <= sizeof (int));
		assert (sizeof (int16_t) <= sizeof (int));

		if (flag & STR_UNSIGNED)
		{
			switch (size)
			{
			case 8:  value = (intmax_t) va_arg (va, unsigned); break;
			case 16: value = (intmax_t) va_arg (va, unsigned); break;
			case 32: value = (intmax_t) va_arg (va, uint32_t); break;
			case 64: value = (intmax_t) va_arg (va, uint64_t); break;
			}
		}
		else if (flag & STR_SIGNED)
		{
			switch (size)
			{
			case 8:  value = (intmax_t) va_arg (va, int); break;
			case 16: value = (intmax_t) va_arg (va, int); break;
			case 32: value = (intmax_t) va_arg (va, int32_t); break;
			case 64: value = (intmax_t) va_arg (va, int64_t); break;
			}
		}

		switch (*f)
		{
		case '_':
			f ++;
			base = str_to_i32 (&f, 10);
			break;
		}

		str_from_imax (o, n, value, (int8_t) base, '.');
	}
end:
	va_end (va);
	return;
}
