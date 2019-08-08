#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <io.h>
#include <unistd.h>
#include <sys/types.h>

#include <time.h>
#include <uv.h>

#include <iup.h>
//#include "Scintilla.h"
#include <iup_scintilla.h>
#include <iup_config.h>

#include <csc_debug.h>
#include <csc_debug_uv.h>
#include <csc_malloc_file.h>
#include <csc_fspath.h>
#include <csc_basic.h>
#include <csc_str.h>

#include "ide_images.h"
#include "sci.h"

static Ihandle * gih_tree = NULL;
static Ihandle * gih_sci = NULL;


int main(int argc, char* argv[])
{
	setbuf (stdout, NULL);
	setbuf (stderr, NULL);
	IupOpen (&argc, &argv);
	//ide_images_load ();
	IupSetGlobal ("UTF8MODE", "No");

	Ihandle * dlg = NULL;
	Ihandle * menu = NULL;
	IupScintillaOpen();
	gih_sci = IupScintilla();
	IupSetAttribute (gih_sci, "EXPAND", "YES");
	Ihandle * btn = IupButton("Hej",NULL);
	IupSetAttribute (btn, "EXPAND", "YES");

	{
		Ihandle * tools = IupMenu
		(
		IupItem("Open", NULL),
		IupItem("Next", NULL),
		IupItem("IupElementPropertiesDialog", NULL),
		NULL
		);
		menu = IupMenu (IupSubmenu ("Tools", tools), NULL);
	}

	Ihandle * split = IupSplit (gih_sci, btn);

	dlg = IupDialog (IupVbox (split, NULL));
	IupSetAttributeHandle(dlg, "MENU", menu);
	IupSetAttribute(dlg, "TITLE", "Simple Notepad");
	IupSetAttribute(dlg, "SIZE", "QUARTERxQUARTER");
	IupShowXY(dlg, IUP_CENTER, IUP_CENTER);
	IupSetAttribute(dlg, "USERSIZE", NULL);

	sci_setup (gih_sci);


	IupMainLoop();
	IupClose();
	return EXIT_SUCCESS;
}

