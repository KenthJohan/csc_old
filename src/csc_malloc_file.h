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
#include <csc_debug.h>
#include <stdio.h> //fopen
#include <string.h> //memset
#include <stdlib.h> //malloc

char * csc_malloc_file (char const * filename)
{
	ASSERTF (filename != NULL, "filename is NULL%s", "");
	FILE * file = fopen (filename, "rb");
	ASSERTF (file != NULL, "can not open file '%s'", filename);
	int r;
	r = fseek (file, 0, SEEK_END);
	ASSERT (r == 0);
	long length = ftell (file);
	ASSERT (length >= 0);
	r = fseek (file, 0, SEEK_SET);
	ASSERT (r == 0);
	char * buffer = (char *) malloc ((unsigned) length + 1);
	ASSERTF (buffer != NULL, "buffer is NULL%s", "");
	memset (buffer, 0, (unsigned) length + 1);
	//buffer [length + 1] = 0;
	if (length > 0)
	{
		size_t n = fread (buffer, (unsigned) length, 1, file);
		ASSERTF (n == 1, "fread error %i %i", (int)n, (int)length);
	}
	fclose (file);
	return buffer;
}

char * csc_malloc_file1 (char const * filename, long * length)
{
	ASSERTF (filename != NULL, "filename is NULL%s", "");
	FILE * file = fopen (filename, "rb");
	ASSERTF (file != NULL, "can not open file '%s'", filename);
	int r;
	r = fseek (file, 0, SEEK_END);
	ASSERT (r == 0);
	*length = ftell (file);
	ASSERT ((*length) >= 0);
	r = fseek (file, 0, SEEK_SET);
	ASSERT (r == 0);
	char * buffer = (char *) malloc ((unsigned) (*length) + 1);
	ASSERTF (buffer != NULL, "buffer is NULL%s", "");
	memset (buffer, 0, (unsigned) (*length) + 1);
	//buffer [length + 1] = 0;
	if (*length > 0)
	{
		size_t n = fread (buffer, (unsigned) (*length), 1, file);
		ASSERTF (n == 1, "fread error %i %i", (int)n, (int)(*length));
	}
	fclose (file);
	return buffer;
}
