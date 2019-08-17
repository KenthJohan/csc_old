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
#include <iup.h>
#include <csc_debug.h>
#include <stdio.h>
#include <string.h>



void IupGetGlobal_MONITORSINFO (int * x, int * y, int * w, int * h)
{
	char * si = IupGetGlobal ("MONITORSINFO");
	ASSERT (si);
	sscanf (si, "%i %i %i %i", x, y, w, h);
	printf ("%s\n", si);
}

void IupGetGlobal_SCREENSIZE (int * w, int * h)
{
	char * si = IupGetGlobal ("SCREENSIZE");
	ASSERT (si);
	sscanf (si, "%ix%i", w, h);
	printf ("%s\n", si);
}


int IupTree_nleaf (Ihandle * h, int ref)
{
	int n = 0;
	int i = ref;
	int depth = IupGetIntId (h, "DEPTH", ref);
	while (1)
	{
		i ++;
		char * kind = IupGetAttributeId (h, "KIND", i);
		if (kind == NULL) {break;}
		int d = IupGetIntId (h, "DEPTH", i);
		if (depth == d) {break;}

		if (strcmp (kind, "LEAF") == 0)
		{
			n ++;
		}
	}
	return n;
}


size_t IupCopyAttribute (Ihandle* ih, const char* name, char * des, size_t size)
{
	char const * data = IupGetAttribute (ih, name);
	if (data == 0) {return 0;}
	memcpy (des, data, size);
	return size;
}
