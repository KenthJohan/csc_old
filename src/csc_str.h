#pragma once
#include <string.h>


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
		if ((*str1) == '\0') {break;}
		if ((*str2) == '\0') {break;}
		if ((*str1) == (*str2))
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
