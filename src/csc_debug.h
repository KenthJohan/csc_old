/*
MIT License

Copyright (c) 2018 Johan Söderlind Åström

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
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "csc_tcol.h"

//https://stackoverflow.com/questions/8487986/file-macro-shows-full-path
#ifdef _WIN32
#define __RELATIVE_FILE__ (strrchr("\\" __FILE__, '\\') + 1)
#else
#define __RELATIVE_FILE__ (strrchr("/" __FILE__, '/') + 1)
#endif

//https://gcc.gnu.org/onlinedocs/gcc/Variadic-Macros.html
#define ASSERT(A)               if (!(A)) {assert_format (__COUNTER__, __RELATIVE_FILE__, __LINE__, __func__, #A, (0), NULL, NULL                );}
#define ASSERT_F(A, F, ...)     if (!(A)) {assert_format (__COUNTER__, __RELATIVE_FILE__, __LINE__, __func__, #A, (0), NULL,  (F), ## __VA_ARGS__);}
#define ASSERT_C(A, C)          if (!(A)) {assert_format (__COUNTER__, __RELATIVE_FILE__, __LINE__, __func__, #A, (C),   #C, NULL                );}
#define ASSERT_CF(A, C, F, ...) if (!(A)) {assert_format (__COUNTER__, __RELATIVE_FILE__, __LINE__, __func__, #A, (C),   #C,  (F), ## __VA_ARGS__);}

#define TRACE(F)            trace_format (__COUNTER__, __RELATIVE_FILE__, __LINE__, __func__, (0), NULL,  (F)                )
#define TRACE_F(F, ...)     trace_format (__COUNTER__, __RELATIVE_FILE__, __LINE__, __func__, (0), NULL,  (F), ## __VA_ARGS__)
#define TRACE_CF(C, F, ...) trace_format (__COUNTER__, __RELATIVE_FILE__, __LINE__, __func__, (C),   #C,  (F), ## __VA_ARGS__)


__attribute__ ((__unused__))
static void assert_format 
(
	int id, 
	char const * file, 
	int line, 
	char const * fn, 
	char const * exp, 
	int code,
	char const * scode,
	char const * fmt, 
	...
)
{
	va_list list;
	va_start (list, fmt);
	fprintf (stderr, TCOL (TCOL_BOLD, TCOL_RED, TCOL_DEFAULT) "ASSERT %04i" TCOL_RST " ", id);
	fprintf (stderr, TCOL (TCOL_BOLD, TCOL_BLUE, TCOL_DEFAULT) "%s" TCOL_RST ":", file);
	fprintf (stderr, TCOL (TCOL_BOLD, TCOL_BLUE, TCOL_DEFAULT) "%04i" TCOL_RST " in ", line);
	fprintf (stderr, TCOL (TCOL_NORMAL, TCOL_YELLOW , TCOL_DEFAULT) "%s" TCOL_RST " () ", fn);
	fprintf (stderr, TCOL (TCOL_BOLD, TCOL_BLACK, TCOL_RED) "[%s]" TCOL_RST " ", exp);
	if (scode)
	{
		fprintf (stderr, TCOL (TCOL_BOLD, TCOL_BLACK, TCOL_YELLOW) "[%i %s]" TCOL_RST " ", code, scode);
	}
	fprintf (stderr, "[%04i:" TCOL (TCOL_BOLD, TCOL_RED , TCOL_DEFAULT) "%s" TCOL_RST "]: ", errno, strerror (errno));
	vfprintf (stderr, fmt, list);
	fprintf (stderr, "\n");
	fflush (stderr);
	va_end (list);
	exit (1);
}



__attribute__ ((__unused__))
static void trace_format 
(
	int id, 
	char const * file, 
	int line, 
	char const * fn, 
	int code,
	char const * scode,
	char const * fmt, 
	...
)
{
	va_list list;
	va_start (list, fmt);
	
	fprintf (stderr, TCOL (TCOL_BOLD, TCOL_YELLOW, TCOL_DEFAULT) "TRACE %04i" TCOL_RST " ", id);
	fprintf (stderr, TCOL (TCOL_BOLD, TCOL_BLUE, TCOL_DEFAULT) "%s" TCOL_RST ":", file);
	fprintf (stderr, TCOL (TCOL_BOLD, TCOL_BLUE, TCOL_DEFAULT) "%04i" TCOL_RST " in ", line);
	fprintf (stderr, TCOL (TCOL_NORMAL, TCOL_YELLOW , TCOL_DEFAULT) "%s" TCOL_RST " ()", fn);
	if (scode)
	{
		fprintf (stderr, TCOL (TCOL_BOLD, TCOL_BLACK, TCOL_BLUE) " [%i %s]" TCOL_RST " ", code, scode);
	}
	fprintf (stderr, ": ");
	vfprintf (stderr, fmt, list);
	fprintf (stderr, "\n");
	fflush (stderr);
	va_end (list);
}
