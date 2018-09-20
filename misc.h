#pragma once

#include "debug.h"
#include <inttypes.h>
#include <time.h>
#include <stdio.h>




uint32_t str_u32 (char const * str)
{
	uintmax_t value;
	value = strtoumax (str, NULL, 10);
	ASSERT_F (value != UINTMAX_MAX && errno != ERANGE, "strtoumax error%s", "");
	return (uint32_t) value;
}


intmax_t str_imax (char const * str)
{
	intmax_t value;
	value = strtoimax (str, NULL, 10);
	ASSERT_F (value != INTMAX_MAX && errno != ERANGE, "strtoumax error%s", "");
	return value;
}


void print_time (FILE * f, time_t s)
{
	struct tm * t = localtime (&s);
	fprintf 
	(
		f,
		"%04i-%02i-%02i %02i:%02i:%02i ", 
		t->tm_year + 1900,
		t->tm_mon + 1,
		t->tm_mday,
		t->tm_hour,
		t->tm_min,
		t->tm_sec
	);
}



char * app_malloc_file (char const * filename)
{
	ASSERT_F (filename != NULL, "filename is NULL%s", "");
	FILE * file = fopen (filename, "rb");
	ASSERT_F (file != NULL, "file is NULL%s", "");
	fseek (file, 0, SEEK_END);
	int length = ftell (file);
	fseek (file, 0, SEEK_SET);
	char * buffer = (char *) malloc (length + 1);
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
	char * buffer = (char *) malloc (length + 1);
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


