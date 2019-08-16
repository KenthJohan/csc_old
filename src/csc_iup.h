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
