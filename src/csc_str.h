/*
MIT License

Copyright (c) 2019 CSC Project

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdint.h>

#define CSC_STRNCMP_LITERAL(str1,str2) strncmp ((str1), (str2), sizeof ((str2))-1)

/*
returns positive when (str1) contains substring of (str2) seperated by (delimiters).
returns 0 otherwise.
eg:
	char * ext = strrchr (filename, '.');
	size_t n = csc_str_contains1 (ext, ".txt, .png ,.json", ", ");
*/
size_t csc_str_contains1 (char const * str1, char const * str2, char const * delimiters)
{
	if (str1 == NULL) {return 0;}
	if (str2 == NULL) {return 0;}
	size_t n = 0;
	while (1)
	{
		str2 += strspn (str2, delimiters);
		int end = ((*str1) == '\0') || ((*str2) == '\0');
		if (end) {break;}
		else if ((*str1) == (*str2))
		{
			str1 ++;
			str2 ++;
			n ++;
		}
		else
		{
			str1 -= n;
			n = 0;
			str2 += strcspn (str2, delimiters);
		}
	}
	return n;
}

/*
eg:
	char * ext1 = strrchr ("C:/docs/hello.txt", '.');
	char * ext0 = strrchr ("C:/docs/hello.txt", '/');
*/


int csc_str_next_cmp (char const ** p, int * col, char const * str)
{
	size_t l = strlen (str);
	int diff = strncmp (*p, str, l);
	if (diff == 0)
	{
		(*p) += l;
		(*col) += l;
		return (int) l;
	}
	return 0;
}


void csc_str_skip (char const ** p, int (*f)(int))
{
	while (f (**p)) {(*p)++;}
}


int csc_isalpha (int c)
{
	return isalpha (c);
}


int csc_isdigit (int c)
{
	return isalpha (c);
}


int csc_isalphadigit (int c)
{
	return isalpha (c) || isdigit (c);
}


int csc_next_indentifer (char const ** p, int * col)
{
	char const * q = (*p);
	csc_str_skip (p, isalpha);
	csc_str_skip (p, csc_isalphadigit);
	ptrdiff_t n = (*p) - q;
	(*col) += n;
	return (int)n;
}


int csc_next_literal (char const ** p, int * col)
{
	char const * q = (*p);
	csc_str_skip (p, isdigit);
	ptrdiff_t n = (*p) - q;
	(*col) += n;
	return (int)n;
}


void csc_str_print_hex_array (char * s, size_t sn, uint8_t * v, size_t vn, char const * format, size_t step)
{
	while (vn--)
	{
		if (sn < step) {break;}
		s += snprintf (s, step, format, *v);
		v ++;
		sn -= step;
	}
}
