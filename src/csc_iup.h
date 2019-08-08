#pragma once
#include <iup.h>
#include <csc_debug.h>
#include <stdio.h>



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
