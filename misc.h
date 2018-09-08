#pragma once

#include "debug.h"



char * app_malloc_file (char * filename)
{
	ASSERT_F (filename != NULL, "filename is NULL%s", "");
	FILE * file = fopen (filename, "rb");
	ASSERT_F (file != NULL, "file is NULL%s", "");
	fseek (file, 0, SEEK_END);
	int length = ftell (file);
	fseek (file, 0, SEEK_SET);
	char * buffer = malloc (length + 1);
	ASSERT_F (buffer != NULL, "buffer is NULL%s", "");
	memset (buffer, 0, length + 1);
	//buffer [length + 1] = 0;
	{
		size_t r = fread (buffer, length, 1, file);
		ASSERT_F (r == 1, "fread error %i %i", (int)r, (int)length);
	}
	fclose (file);
	return buffer;
}


char * app_malloc_from_file (FILE * file)
{
	ASSERT_F (file != NULL, "file is NULL%s", "");
	fseek (file, 0, SEEK_END);
	int length = ftell (file);
	fseek (file, 0, SEEK_SET);
	char * buffer = malloc (length + 1);
	ASSERT_F (buffer != NULL, "buffer is NULL%s", "");
	memset (buffer, 0, length + 1);
	//buffer [length + 1] = 0;
	{
		size_t r = fread (buffer, length, 1, file);
		ASSERT_F (r == 1, "fread error %i %i", (int)r, (int)length);
	}
	fclose (file);
	return buffer;
}


//check if any of needles exists in the haystack
int str_contain (char const * haystack, size_t len, char const * needles)
{
	//rename to shorter variables.
	char const * h = haystack;
	char const * n = needles;
	//return 1 or 0 if we there is no needles to look for?
	if (n[0] == '\0') {return 0;}
	while (1)
	{
		//return false if the maxium search length reached
		if (h == (haystack + len)) {return 0;}
		//return false if end of haystack reached
		if (h[0] == '\0') {return 0;}
		//reset needle and search into next hay if no needle found
		if (n[0] == '\0') {n = needles; h ++; continue;}
		//look for next needle if hay is not a needle.
		if (h[0] != n[0]) {n ++; continue;}
		//return true if hey is needle.
		if (h[0] == n[0]) {return 1;}
	}
}


