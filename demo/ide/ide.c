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
#include <csc_iup.h>

#include "img.h"
#include "sci.h"
#include "fstree.h"



static struct
{
	Ihandle * dlg;
	Ihandle * sci;
	Ihandle * tree1;
} gapp;



static int sci_cb_kany (Ihandle *ih, int c)
{
	//  printf("K_ANY(key: %d)\n", c);
	if (c == K_cS) {return IUP_IGNORE;}
	if (c == K_cO)
	{
		IupSetAttribute(ih, "OVERWRITE", "ON");
		return IUP_IGNORE;
	}
	return IUP_CONTINUE;
}

static int sci_cb_marginclick (Ihandle *self, int margin, int line, char* status)
{
	(void)status;
	(void)line;
	//printf("MARGINCLICK_CB(Margin: %d, Line: %d, Status:%s)\n", margin, line, status);
	//printf("Fold Level = %s\n", IupGetAttributeId(self, "FOLDLEVEL", line));
	int mark = IupGetIntId (self, "MARKERGET", line);
	//printf ("MARKERGET %x\n", mark);

	if (margin == APP_SCIMARGIN_FOLD)
	{
		IupSetfAttribute (self, "FOLDTOGGLE", "%d", line);
	}

	if (margin == APP_SCIMARGIN_BM0)
	{
		if (mark & (1 << 4)) {IupSetIntId (self, "MARKERDELETE", line, 4);}
		else {IupSetIntId (self, "MARKERADD", line, 4);}
	}

	if (margin == APP_SCIMARGIN_BM1)
	{
		if (mark & (1 << 5)) {IupSetIntId (self, "MARKERDELETE", line, 5);}
		else {IupSetIntId (self, "MARKERADD", line, 5);}
	}

	if (margin == APP_SCIMARGIN_BM2)
	{
		if (mark & (1 << 6)) {IupSetIntId (self, "MARKERDELETE", line, 6);}
		else {IupSetIntId (self, "MARKERADD", line, 6);}
	}

	return IUP_DEFAULT;
}

static int sci_cb_hotspotclick (Ihandle *self, int pos, int line, int col, char* status)
{
	char *text = IupGetAttributeId(self, "LINE", line);
	printf("HOTSPOTCLICK_CB (Pos: %d, Line: %d, Col: %d, Status:%s)\n", pos, line, col, status);
	printf("    line text = %s\n", text);
	return IUP_DEFAULT;
}

static int sci_cb_button (Ihandle* self, int button, int pressed, int x, int y, char* status)
{
	printf("BUTTON_CB = button: %d, pressed: %d, x: %d, y: %d, status: %s\n", button, pressed, x, y, status);
	(void)self;
	return IUP_DEFAULT;
}

/*
static int motion_cb (Ihandle *self, int x, int y, char *status)
{
	//printf("MOTION_CB = x: %d, y: %d, status: %s\n", x, y, status);
	(void)self;
	return IUP_DEFAULT;
}
*/

static int sci_cb_caret (Ihandle *self, int lin, int col, int pos)
{
	printf("CARET_CB = lin: %d, col: %d, pos: %d\n", lin, col, pos);
	(void)self;
	return IUP_DEFAULT;
}

static int sci_cb_valuechanged (Ihandle *self)
{
	printf("VALUECHANGED_CB\n");
	(void)self;
	return IUP_DEFAULT;
}

static int sci_cb_action (Ihandle *self, int insert, int pos, int length, char* text)
{
	printf("ACTION = insert: %d, pos: %d, lenght:%d, text: %s\n", insert, pos, length, text);
	(void)self;
	return IUP_IGNORE;
}

static int sci_cb_next_line (Ihandle* ih)
{
	(void)ih;
	printf ("btn_next_action!\n");
	static int line = 0;
	//IupSetIntId (gih_sci, "MARKERDELETE", line, 8);
	line ++;
	IupSetIntId (gapp.sci, "MARKERADD", line, 8);
	IupSetAttribute (gapp.sci, "APPEND", "APPEND");
	return IUP_DEFAULT;
}


int sci_load (Ihandle * h)
{
	(void) h;
	int id = IupGetInt (gapp.tree1, "VALUE");
	char const * title;
	struct fsnode * node;
	title = IupGetAttributeId (gapp.tree1, "TITLE", id);
	if (title == NULL) {return IUP_DEFAULT;}
	node = IupTreeGetUserId (gapp.tree1, id);
	if (node == NULL) {return IUP_DEFAULT;}
	printf ("USERDATA %i %s\n", id, node->path);
	sci_load_filename (gapp.sci, node->path);
	return IUP_DEFAULT;
}

/*
Action generated when a leaf is to be executed.
This action occurs when the user double clicks a leaf, or hits Enter on a leaf.
*/
int fstree_execute (Ihandle * h, int id)
{
	char const * title;
	struct fsnode * node;
	title = IupGetAttributeId (h, "TITLE", id);
	if (title == NULL) {return IUP_DEFAULT;}
	node = IupTreeGetUserId (h, id);
	if (node == NULL) {return IUP_DEFAULT;}
	printf ("USERDATA %i %s\n", id, node->path);
	char * ext = strrchr (title, '.');
	if (ext && strcmp (ext, ".gcov") == 0)
	{
		sci_gcov_filename (gapp.sci, node->path);
	}
	else
	{
		sci_load_filename (gapp.sci, node->path);
	}
	return IUP_DEFAULT;
}



/*
User left clicks on refresh button from fstree right click menu.
*/
int fstree_cb_refresh (void)
{
	fstree_update (gapp.tree1);
	return IUP_DEFAULT;
}


int fstree_cb_gcov_putlabel (void)
{
	int id = IupGetInt (gapp.tree1, "VALUE");
	fstree_gcov_putlabel (gapp.tree1, id);
	return IUP_DEFAULT;
}


int fstree_cb_extfilter (void)
{
	char extw [100] = {0};
	IupCopyAttribute (gapp.tree1, "FSTREE_EXTW", extw, 100);
	int r = IupGetParam ("Whitelist file extensions", NULL, 0, "File extensions:%s\n", extw, NULL);
	if (r != 1) {return IUP_DEFAULT;}
	IupSetStrAttribute (gapp.tree1, "FSTREE_EXTW", extw);
	fstree_update (gapp.tree1);
	return IUP_DEFAULT;
}




/*
User right click on the fstree.
Node (id) is the closest to mouse pointer.
*/
int fstree_cb_rclick (Ihandle* h, int id)
{
	char * kind = IupGetAttributeId (h, "KIND", id);
	ASSERT (kind);
	Ihandle * menu = NULL;
	if (id == 0)
	{
		IupSetInt (h, "VALUE", id);
		menu = IupMenu
		(
		IupItem ("Refresh", "refresh"),
		IupItem ("Passfilter by extensions", "extfilter"),
		NULL
		);
		IupSetFunction ("extfilter", (Icallback) fstree_cb_extfilter);
		IupSetFunction ("refresh", (Icallback) fstree_cb_refresh);
		IupPopup (menu, IUP_MOUSEPOS, IUP_MOUSEPOS);
		IupDestroy (menu);
	}
	else if (strcmp (kind, "BRANCH") == 0)
	{
		IupSetInt (h, "VALUE", id);
		menu = IupMenu
		(
		IupItem ("gcov", "gcov"),
		NULL
		);
		IupSetFunction ("gcov", (Icallback) fstree_cb_gcov_putlabel);
		IupPopup (menu, IUP_MOUSEPOS, IUP_MOUSEPOS);
		IupDestroy (menu);
	}
	else if (strcmp (kind, "LEAF") == 0)
	{
		IupSetInt (h, "VALUE", id);
		menu = IupMenu
		(
		IupItem ("source", "source"),
		NULL
		);
		IupSetFunction ("source", (Icallback) sci_load);
		IupPopup (menu, IUP_MOUSEPOS, IUP_MOUSEPOS);
		IupDestroy (menu);
	}

	return IUP_DEFAULT;
}

static int main_cb_exit (void)
{
	return IUP_CLOSE;
}

static int main_cb_show_layoutdlg (Ihandle* ih)
{
	IupShow (IupLayoutDialog (gapp.dlg));
	(void)ih;
	return IUP_DEFAULT;
}





int main(int argc, char* argv[])
{
	setbuf (stdout, NULL);
	setbuf (stderr, NULL);
	IupOpen (&argc, &argv);
	IupScintillaOpen ();
	ide_images_load ();
	IupSetGlobal ("UTF8MODE", "No");


	gapp.tree1 = IupTree ();
	gapp.sci = IupScintilla ();
	gapp.dlg = IupDialog (IupVbox (IupSplit (gapp.tree1, gapp.sci), NULL));
	IupSetHandle ("tree1", gapp.tree1);

	{
		IupSetAttribute (gapp.tree1, "BORDER", "NO");
		IupSetAttribute (gapp.tree1, "EXPAND", "Yes");
		IupSetAttribute (gapp.tree1, "TITLE", "root1");
		IupSetCallback (gapp.tree1, "EXECUTELEAF_CB", (Icallback) fstree_execute);
		IupSetCallback (gapp.tree1, "RIGHTCLICK_CB", (Icallback) fstree_cb_rclick);
	}

	{
		IupSetAttribute (gapp.sci, "VISIBLECOLUMNS", "80");
		IupSetAttribute (gapp.sci, "VISIBLELINES", "40");
		IupSetAttribute (gapp.sci, "SCROLLBAR", "YES");
		IupSetAttribute (gapp.sci, "BORDER", "NO");
		IupSetAttribute (gapp.sci, "EXPAND", "Yes");
		IupSetAttribute (gapp.sci, "OVERWRITE", "ON");
		IupSetCallback (gapp.sci, "MARGINCLICK_CB", (Icallback)sci_cb_marginclick);
		IupSetCallback (gapp.sci, "HOTSPOTCLICK_CB", (Icallback)sci_cb_hotspotclick);
		IupSetCallback (gapp.sci, "BUTTON_CB", (Icallback)sci_cb_button);
		//IupSetCallback (handle_sci, "MOTION_CB", (Icallback)motion_cb);
		IupSetCallback (gapp.sci, "K_ANY", (Icallback)sci_cb_kany);
		IupSetCallback (gapp.sci, "CARET_CB", (Icallback)sci_cb_caret);
		IupSetCallback (gapp.sci, "VALUECHANGED_CB", (Icallback)sci_cb_valuechanged);
		IupSetCallback (gapp.sci, "ACTION", (Icallback)sci_cb_action);
	}

	{
		struct
		{
			Ihandle * menu;
			Ihandle * menu1;
			Ihandle * next;
			Ihandle * show_layoutdlg;
			Ihandle * exit;
		} menu;
		menu.next = IupItem ("Next", NULL);
		menu.show_layoutdlg = IupItem ("IupLayoutDialog", NULL);
		menu.exit = IupItem ("Exit", NULL);
		menu.menu1 = IupMenu (menu.next, menu.show_layoutdlg, IupSeparator(), menu.exit, NULL);
		menu.menu = IupMenu (IupSubmenu ("Menu", menu.menu1), NULL);
		IupSetCallback (menu.next, "ACTION", (Icallback) sci_cb_next_line);
		IupSetCallback (menu.show_layoutdlg, "ACTION", (Icallback) main_cb_show_layoutdlg);
		IupSetCallback (menu.exit, "ACTION", (Icallback) main_cb_exit);
		IupSetAttributeHandle(gapp.dlg, "MENU", menu.menu);
		IupSetAttribute (gapp.dlg, "TITLE", "gcovenant");
		IupSetAttribute (gapp.dlg, "RASTERSIZE", "700x500");
		IupSetAttribute (gapp.dlg, "MARGIN", "10x10");
		IupSetAttribute (gapp.dlg, "RASTERSIZE", NULL);
		IupShow (gapp.dlg);
	}

	IupSetStrAttribute (gapp.tree1, "FSTREE_ROOT", "..");
	IupSetStrAttribute (gapp.tree1, "FSTREE_EXTW", ".c .h .gcov");
	fstree_update (gapp.tree1);
	sci_setup (gapp.sci);

	IupMainLoop();
	IupClose();
	return EXIT_SUCCESS;
}

