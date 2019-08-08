#pragma once
#include <csc_debug.h>
#include <string.h> //strstr
#include <ctype.h> //isalpha



int csc_fspath_ishidden (char const * s)
{
	if (*s != '.') {return 0;}
	s ++;
	if (isalpha (*s) || isdigit (*s)) {return 1;}
	return 0;
}

int csc_fspath_ishidden2 (char const * s)
{
	while (1)
	{
		s = strstr (s, "/.");
		if (s == NULL) {break;}
		s += 2;
		if (isalpha (*s) || isdigit (*s)) {return 1;}
	}
	return 0;
}
