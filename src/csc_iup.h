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
#include <stdio.h>
#include <string.h>

#include "csc_debug.h"


static void IupGetGlobal_MONITORSINFO (int * x, int * y, int * w, int * h)
{
	char * si = IupGetGlobal ("MONITORSINFO");
	ASSERT (si);
	sscanf (si, "%i %i %i %i", x, y, w, h);
	printf ("%s\n", si);
}


static void IupGetGlobal_SCREENSIZE (int * w, int * h)
{
	char * si = IupGetGlobal ("SCREENSIZE");
	ASSERT (si);
	sscanf (si, "%ix%i", w, h);
	printf ("%s\n", si);
}


static int IupTree_nleaf (Ihandle * h, int ref)
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


static size_t IupCopyAttribute (Ihandle* ih, const char* name, char * des, size_t size)
{
	char const * data = IupGetAttribute (ih, name);
	if (data == 0) {return 0;}
	memcpy (des, data, size);
	return size;
}


static void IupItem_str1 (Ihandle * item [], char const * name, int n, int step, char const * idname, char const * callback_name)
{
	ASSERT (name);
	while (n--)
	{
		item [n] = IupItem (name, callback_name);
		IupSetInt (item [n], idname, n);
		name += step;
	}
}


static void IupItem_str1_append (Ihandle * menu, char const * name, int n, int step, char const * idname, char const * callback_name)
{
	ASSERT (name);
	while (n--)
	{
		Ihandle * item = IupItem (name, callback_name);
		IupSetInt (item, idname, n);
		IupAppend (menu, item);
		IupMap (item);
		name += step;
	}
}


static void IupDestroy_children (Ihandle * base)
{
	Ihandle * child = IupGetNextChild(base, NULL);
	while (child)
	{
		//printf ("Type=%s, title=(%s)\n", IupGetClassName(base), IupGetAttribute(base, "TITLE"));
		IupDestroy (child);
		child = IupGetNextChild (base, NULL);
	}
}


static void csc_Iup_print (Ihandle * base, char const * attribute, FILE * f)
{
	base = IupGetNextChild (base, NULL);
	while (base)
	{
		fprintf (f, "type %s title %s\n", IupGetClassName (base), IupGetAttribute (base, attribute));
		base = IupGetNextChild (NULL, base);
	}
}

void csc_Iup_print_all_attributes (Ihandle *handle)
{
	char * attr [100] = {NULL};
	int n = IupGetAllAttributes (handle, attr, 100);
	for (int i = 0; i < n; ++i)
	{
		printf ("%04i %s\n", i, attr [i]);
	}
}



void csc_Iup_set_string_list (Ihandle *self, char const * data, size_t step, size_t n)
{
	for (size_t i = 0; i < n; ++i)
	{
		//puts (data + step * i);
		IupSetAttributeId (self, "", (int)i+1, data + step * i);
	}
}
