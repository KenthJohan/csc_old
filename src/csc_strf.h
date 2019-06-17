#pragma once
#include <stdint.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include <csc_basic.h>

#define STR_SET_0Z "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"
#define STR_SET_Z0 "ZYXWVUTSRQPONMLKJIHGFEDCBA9876543210"
#define STR_INB(x, a, b, base) IN ((x), (a), MIN ((base) + (a), (b)))
#define STR_SIGNED (1 << 0)
#define STR_UNSIGNED (1 << 1)

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

int8_t str_to_i8 (char const ** f, int base)
{
	int32_t v = str_to_i32 (f, base);
	return (int8_t) v;
}

uint8_t str_to_u8 (char const ** f, int base)
{
	uint32_t v = str_to_u32 (f, base);
	return (uint8_t) v;
}

void str_rev (char * o, uint32_t n)
{
	char * e = o + n;
	n /= 2;
	while (n--)
	{
		e --;
		SWAPX (*o, *e);
		o ++;
	}
}

void str_rep (char * o, uint32_t n, char pad)
{
	while (n--)
	{
		*o = pad;
		o ++;
	}
}

void str_from_imax (char * o, uint32_t n, intmax_t value, int base, char pad)
{
	assert (o != NULL);
	assert (base != 0);
	assert (base < (int8_t)sizeof (STR_SET_0Z));
	int rem;
	int negative = 0;
	if (n == 0) {return;}
	o += n;
	if (value < 0)
	{
		value = -value;
		negative = 1;
	}
	while (1)
	{
		if (n == 0) {break;}
		rem = value % base;
		value /= base;
		if (rem < 0)
		{
			rem += abs (base);
			value += 1;
		}
		o --;
		n --;
		*o = STR_SET_0Z [rem];
		if (value == 0) {break;}
	}
	if (n > 0 && negative)
	{
		o --;
		n --;
		*o = '-';
	}
	while (1)
	{
		if (n == 0) {break;}
		o --;
		n --;
		*o = pad;
	}
	return;
}

uint32_t str_from_imax2 (char * o, uint32_t n, intmax_t value, int base)
{
	assert (o != NULL);
	assert (base != 0);
	assert (base < (int8_t)sizeof (STR_SET_0Z));
	int rem;
	uint32_t m = 0;
	if (m >= n) {return m;}
	if (value < 0)
	{
		value = -value;
		*o = '-';
		o ++;
		m ++;
	}
	else
	{
		*o = '+';
		o ++;
		m ++;
	}
	while (1)
	{
		if (m >= n) {break;}
		rem = value % base;
		value /= base;
		if (rem < 0)
		{
			rem += abs (base);
			value += 1;
		}
		*o = STR_SET_0Z [rem];
		o ++;
		m ++;
		if (value == 0) {break;}
	}
	str_rev (o-m+1, m-1);
	return m;
}




//format specifier
//%[width][u,i][size]_[base]
void str_fmtv (char * o, uint32_t n, char const * f, va_list va)
{
	uint32_t flag = 0;
	uint32_t size = 0;
	uint32_t width = 0;
	int base;
	intmax_t value = 0;
	while (1)
	{
		//Look for format specifier starting with '%'
		switch (*f)
		{
		case '\0':
			goto end;
		case '%':
			f ++;
			break;
		default:
			*o = *f;
			o ++;
			n --;
			f ++;
			continue;
		}

		//Look if width is specified
		if (IN (*f, '0', '9'))
		{
			width = str_to_u32 (&f, 10);
		}

		//Look for signed or unsigned
		//Look for size
		switch (*f)
		{
		case 'u':
			f ++;
			flag |= STR_UNSIGNED;
			break;
		case 'i':
			f ++;
			flag |= STR_SIGNED;
			break;
		}

		if (IN (*f, '0', '9'))
		{
			size = str_to_u32 (&f, 10);
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
			base = (int) str_to_i32 (&f, 10);
			break;
		default:
			base = 10;
			break;
		}

		uint32_t m = str_from_imax2 (o, n, value, base);
		o += m;
		n -= m;
		if (width > m)
		{
			width = MIN (n, width - m);
		}
		str_rep (o, width, '.');
		o += width;
		n -= width;
	}
end:
	return;
}

void str_fmt (char * o, uint32_t n, char const * f, ...)
{
	va_list va;
	va_start (va, f);
	str_fmtv (o, n, f, va);
	va_end (va);
}

void str_printf (char const * f, ...)
{
	va_list va;
	va_start (va, f);
	char buf [100] = {'\0'};
	str_fmtv (buf, 100, f, va);
	puts (buf);
	va_end (va);
}
