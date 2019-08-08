#pragma once
#include <csc_debug.h>
#include <stdio.h> //fopen
#include <string.h> //memset
#include <stdlib.h> //malloc

char * csc_malloc_file (char const * filename)
{
	ASSERT_F (filename != NULL, "filename is NULL%s", "");
	FILE * file = fopen (filename, "rb");
	ASSERT_F (file != NULL, "file is NULL%s", "");
	int r;
	r = fseek (file, 0, SEEK_END);
	ASSERT (r == 0);
	long length = ftell (file);
	ASSERT (length >= 0);
	r = fseek (file, 0, SEEK_SET);
	ASSERT (r == 0);
	char * buffer = (char *) malloc ((unsigned) length + 1);
	ASSERT_F (buffer != NULL, "buffer is NULL%s", "");
	memset (buffer, 0, (unsigned) length + 1);
	//buffer [length + 1] = 0;
	if (length > 0)
	{
		size_t n = fread (buffer, (unsigned) length, 1, file);
		ASSERT_F (n == 1, "fread error %i %i", (int)n, (int)length);
	}
	fclose (file);
	return buffer;
}
