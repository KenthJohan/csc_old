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



static struct
{
	Ihandle * dlg;
	Ihandle * sci;
	Ihandle * tree1;
	Ihandle * tree2;
	Ihandle * zbox;
} gapp;



static int k_any (Ihandle *ih, int c)
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

static int marginclick_cb (Ihandle *self, int margin, int line, char* status)
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

static int hotspotclick_cb (Ihandle *self, int pos, int line, int col, char* status)
{
	char *text = IupGetAttributeId(self, "LINE", line);
	printf("HOTSPOTCLICK_CB (Pos: %d, Line: %d, Col: %d, Status:%s)\n", pos, line, col, status);
	printf("    line text = %s\n", text);
	return IUP_DEFAULT;
}

static int button_cb (Ihandle* self, int button, int pressed, int x, int y, char* status)
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

static int caret_cb (Ihandle *self, int lin, int col, int pos)
{
	printf("CARET_CB = lin: %d, col: %d, pos: %d\n", lin, col, pos);
	(void)self;
	return IUP_DEFAULT;
}

static int valuechanged_cb (Ihandle *self)
{
	printf("VALUECHANGED_CB\n");
	(void)self;
	return IUP_DEFAULT;
}

static int action_cb (Ihandle *self, int insert, int pos, int length, char* text)
{
	printf("ACTION = insert: %d, pos: %d, lenght:%d, text: %s\n", insert, pos, length, text);
	(void)self;
	return IUP_IGNORE;
}

static int gapp_view_tree2 (Ihandle * h)
{
	(void)h;
	Ihandle * zbox = IupGetHandle ("zbox");
	ASSERT (zbox);
	IupSetAttribute (zbox, "VALUE", "tree2");
	return IUP_DEFAULT;
}

static int gapp_view_tree1 (Ihandle * h)
{
	(void)h;
	Ihandle * zbox = IupGetHandle ("zbox");
	ASSERT (zbox);
	IupSetAttribute (zbox, "VALUE", "tree1");
	return IUP_DEFAULT;
}

static int btn_next_action (Ihandle* ih)
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

static int btn_prop_action (Ihandle* ih)
{
	IupShow (IupElementPropertiesDialog (gapp.sci));
	(void)ih;
	return IUP_DEFAULT;
}

/*
Walk directory recursive.
Build directory tree in IupTree.
*/
void fstree_build (Ihandle * ih, char * dir0, int ref)
{
	//printf ("%i %s\n", id, dir0);
	//Set to NO to add many items to the tree without updating the display. Default: "YES".
	IupSetAttribute(ih, "AUTOREDRAW", "No");
	struct _finddata_t fileinfo;
	intptr_t handle;
	char star [MAX_PATH];
	snprintf (star, MAX_PATH, "%s/*", dir0);
	//printf ("dir %i %s\n", i, star);
	handle = _findfirst (star, &fileinfo);
	if(handle == -1)
	{
		//perror ("Error searching for file");
		//exit (1);
		return;
	}

	while (1)
	{
		char * ext = strrchr (fileinfo.name, '.');
		if(strcmp(fileinfo.name, ".") == 0 || strcmp(fileinfo.name, "..") == 0){}
		else if (csc_fspath_ishidden (fileinfo.name)){}
		else if ((fileinfo.attrib & _A_SUBDIR) == 0 && csc_str_contains1 (ext, ".c .h .gcov", " "))
		{
			char buf [MAX_PATH];
			snprintf (star, MAX_PATH, "%s/%s", dir0, fileinfo.name);
			snprintf (buf, MAX_PATH, "%i %s/%s", ref, dir0, fileinfo.name);
			//puts (buf);
			//printf ("F %x %s\n", fileinfo.attrib, star);
			IupSetAttributeId (ih, "ADDLEAF", ref, star);
			//IupSetAttributeId (ih, "USERDATA", IupGetInt(ih, "LASTADDNODE"), "Leaf");
		}
		else if (fileinfo.attrib & _A_SUBDIR)
		{
			char buf [MAX_PATH];
			snprintf (star, MAX_PATH, "%s/%s", dir0, fileinfo.name);
			//snprintf (buf, MAX_PATH, "%i %s/%s", ref, dir0, fileinfo.name);
			snprintf (buf, MAX_PATH, "Hej %s", fileinfo.name);
			//puts (buf);
			//printf ("D %x %s\n", fileinfo.attrib, star);
			IupSetAttributeId (ih, "ADDBRANCH", ref, star);
			//IupSetAttributeId (ih, "USERDATA", IupGetInt(ih, "LASTADDNODE"), buf);
			fstree_build (ih, star, ref + 1);
		}
		int r = _findnext (handle, &fileinfo);
		if (r != 0)
		{
			break;
		}
	}
	_findclose(handle);
	IupSetAttribute(ih, "AUTOREDRAW", "Yes");
}


void fstree_icon (Ihandle * ih)
{
	int i = 0;
	while (1)
	{
		//IupSetAttributeId (ih, "USERDATA", i, "Hej");
		char * title = IupGetAttributeId (ih, "TITLE", i);
		char * ud = IupGetAttributeId (ih, "USERDATA", i);
		printf ("%s %s\n", title, ud);
		if (title == NULL) {break;}
		char * ext = strrchr (title, '.');
		if (ext == NULL) {}
		else if (ext && (strcmp (ext, ".a") == 0))
		{
			IupSetAttributeHandleId (ih, "IMAGE", i, gih_img_lib);
		}
		else if (ext && (strcmp (ext, ".h") == 0))
		{
			IupSetAttributeHandleId (ih, "IMAGE", i, gih_img_h);
		}
		else if (ext && (strcmp (ext, ".hpp") == 0))
		{
			IupSetAttributeHandleId (ih, "IMAGE", i, gih_img_hpp);
		}
		else if (ext && (strcmp (ext, ".c") == 0))
		{
			IupSetAttributeHandleId (ih, "IMAGE", i, gih_img_c);
		}
		else if (ext && (strcmp (ext, ".cpp") == 0))
		{
			IupSetAttributeHandleId (ih, "IMAGE", i, gih_img_cpp);
		}
		else if (ext && (strcmp (ext, ".gcov") == 0))
		{
			IupSetAttributeHandleId (ih, "IMAGE", i, gih_img_gcov);
		}
		i ++;
	}
}


/*
Update directories and file names for.
Using globals (gih_tree)
*/
void fstree_refresh_id (Ihandle * ih, int id)
{
	char * dir = IupGetAttributeId (gapp.tree1, "TITLE", id);
	printf ("list1_refresh %s\n", dir);
	IupSetAttributeId (ih, "DELNODE", id, "CHILDREN");
	fstree_build (ih, dir, id);
}



void fstree_label_filename (Ihandle * ih, char const * str1)
{
	//Set to NO to add many items to the tree without updating the display. Default: "YES".
	IupSetAttribute(ih, "AUTOREDRAW", "No");
	char const * ext1 = strrchr (str1, '.');
	if (ext1 == NULL) {return;}
	char const * ext0 = strrchr (str1, '/');
	if (ext0 == NULL) {ext0 = str1;}
	else {ext0 ++;}
	char name [MAX_PATH] = {0};
	strncpy (name, ext0, (unsigned) MIN (ext1-ext0, MAX_PATH));
	printf ("%s\n", name);

	int i = 0;
	while (1)
	{
		char * title = IupGetAttributeId (ih, "TITLE", i);
		if (title == NULL) {break;}
		if (strstr (title, name))
		{
			IupSetAttributeId (ih, "COLOR", i, "0 120 0");
			IupSetAttributeId (ih, "TITLEFONTSTYLE", i, "Bold");
		}
		i ++;
	}
	IupSetAttribute (ih, "AUTOREDRAW", "Yes");
}

/*
Find all gcov files in the IupTree (ih) starting from node (id)
*/
void fstree_label_id (Ihandle * ih, int id)
{
	int i;

	i = 0;
	while (1)
	{
		char * title = IupGetAttributeId (ih, "TITLE", i);
		if (title == NULL) {break;}
		IupSetAttributeId (ih, "COLOR", i, "0 0 0");
		IupSetAttributeId (ih, "TITLEFONTSTYLE", i, "Normal");
		//IupSetAttributeId (ih, "NODEVISIBLE", i, "No");
		i ++;
	}

	i = id;
	while (1)
	{
		char * title = IupGetAttributeId (ih, "TITLE", i);
		char * ext = strrchr (title, '.');
		if (ext && strcmp (ext, ".gcov") == 0)
		{
			fstree_label_filename (ih, title);
			//printf ("title %i %s\n", i, title);
		}
		if (IupGetAttributeId (ih, "NEXT", i) == NULL) {return;}
		i ++;
	}
}

int sci_gcov_filename (Ihandle * h, char const * filename)
{
	IupSetAttribute (h, "APPENDNEWLINE", "No");
	IupSetAttribute (h, "CLEARALL", NULL);
	FILE * f = fopen (filename, "r");
	if (f == NULL) {return IUP_DEFAULT;}
	char line [2048] = {0};
	while (fgets(line, 2048, f))
	{
		int l = IupGetInt (h, "LINECOUNT");
		if (CSC_STRNCMP_LITERAL (line, "        -:    0:") == 0) {continue;}
		IupSetAttribute (h, "APPEND", line+16);
		if (CSC_STRNCMP_LITERAL (line, "    #####") == 0)
		{
			IupSetIntId (h, "MARKERADD", l-1, 8);
		}
		else if (CSC_STRNCMP_LITERAL (line, "        -:") == 0)
		{

		}
		else
		{
			IupSetIntId (h, "MARKERADD", l-1, 9);
		}
	}
	fclose (f);

	return IUP_DEFAULT;
}

int sci_load_filename (Ihandle * h, char const * filename)
{
	char * text = csc_malloc_file (filename);
	if (text == NULL) {return IUP_DEFAULT;}
	//printf ("%s\n", text);
	IupSetAttribute (h, "CLEARALL", NULL);
	IupSetAttribute (h, "INSERT0", text);
	free (text);
	return IUP_DEFAULT;
}

int sci_load (Ihandle * h)
{
	int id = IupGetInt (gapp.tree1, "VALUE");
	char * title = IupGetAttributeId (gapp.tree1, "TITLE", id);
	if (title == NULL) {return IUP_DEFAULT;}
	sci_load_filename (gapp.sci, title);
	return IUP_DEFAULT;
}

/*
Action generated when a leaf is to be executed.
This action occurs when the user double clicks a leaf, or hits Enter on a leaf.
*/
int fstree_execute (Ihandle *ih, int id)
{
	char const * title = IupGetAttributeId (ih, "TITLE", id);
	if (title == NULL) {return IUP_DEFAULT;}
	char const * ud = IupGetAttributeId (ih, "USERDATA", id);
	if (ud) {puts (ud);}
	sci_gcov_filename (gapp.sci, title);
	return IUP_DEFAULT;
}



/*
User left clicks on refresh button from fstree right click menu.
*/
int fstree_refresh (void)
{
	int id = IupGetInt (gapp.tree1, "VALUE");
	fstree_refresh_id (gapp.tree1, id);
	return IUP_DEFAULT;
}

int fstree_label (void)
{
	int id = IupGetInt (gapp.tree1, "VALUE");
	fstree_label_id (gapp.tree1, id);
	return IUP_DEFAULT;
}

/*
User right click on the fstree.
Node (id) is the closest to mouse pointer.
*/
int iupfs_on_rclick (Ihandle* h, int id)
{
	char * kind = IupGetAttributeId (h, "KIND", id);
	ASSERT (kind);
	if (strcmp (kind, "BRANCH") == 0)
	{
		IupSetInt (h, "VALUE", id);
		Ihandle *popup_menu = IupMenu
		(
		IupItem ("Refresh", "refresh"),
		IupItem ("gcov", "gcov"),
		NULL
		);
		IupSetFunction ("refresh", (Icallback) fstree_refresh);
		IupSetFunction ("gcov", (Icallback) fstree_label);
		IupPopup (popup_menu, IUP_MOUSEPOS, IUP_MOUSEPOS);
		IupDestroy (popup_menu);
	}

	if (strcmp (kind, "LEAF") == 0)
	{
		IupSetInt (h, "VALUE", id);
		Ihandle *popup_menu = IupMenu
		(
		IupItem ("update", "update"),
		IupItem ("source", "source"),
		NULL
		);
		IupSetFunction ("source", (Icallback) sci_load);
		IupPopup (popup_menu, IUP_MOUSEPOS, IUP_MOUSEPOS);
		IupDestroy (popup_menu);
	}

	return IUP_DEFAULT;
}

int exit_cb (void)
{
	return IUP_CLOSE;
}


int main(int argc, char* argv[])
{
	setbuf (stdout, NULL);
	setbuf (stderr, NULL);
	IupOpen (&argc, &argv);
	IupScintillaOpen ();
	ide_images_load ();
	IupSetGlobal ("UTF8MODE", "No");


	gapp.tree1 = IupTree ();IupSetHandle ("tree1", gapp.tree1);
	gapp.tree2 = IupTree ();IupSetHandle ("tree2", gapp.tree2);
	gapp.sci = IupScintilla ();
	gapp.zbox = IupZbox (gapp.tree1, gapp.tree2, NULL);IupSetHandle ("zbox", gapp.zbox);
	gapp.dlg = IupDialog (IupVbox (IupSplit (gapp.zbox, gapp.sci), NULL));

	{
		IupSetAttribute (gapp.tree1, "BORDER", "NO");
		IupSetAttribute (gapp.tree1, "EXPAND", "Yes");
		IupSetCallback (gapp.tree1, "EXECUTELEAF_CB", (Icallback) fstree_execute);
		IupSetCallback (gapp.tree1, "RIGHTCLICK_CB", (Icallback) iupfs_on_rclick);
	}

	{
		IupSetAttribute (gapp.sci, "VISIBLECOLUMNS", "80");
		IupSetAttribute (gapp.sci, "VISIBLELINES", "40");
		IupSetAttribute (gapp.sci, "SCROLLBAR", "YES");
		IupSetAttribute (gapp.sci, "BORDER", "NO");
		IupSetAttribute (gapp.sci, "EXPAND", "Yes");
		IupSetAttribute (gapp.sci, "OVERWRITE", "ON");
		IupSetCallback (gapp.sci, "MARGINCLICK_CB", (Icallback)marginclick_cb);
		IupSetCallback (gapp.sci, "HOTSPOTCLICK_CB", (Icallback)hotspotclick_cb);
		IupSetCallback (gapp.sci, "BUTTON_CB", (Icallback)button_cb);
		//IupSetCallback (handle_sci, "MOTION_CB", (Icallback)motion_cb);
		IupSetCallback (gapp.sci, "K_ANY", (Icallback)k_any);
		IupSetCallback (gapp.sci, "CARET_CB", (Icallback)caret_cb);
		IupSetCallback (gapp.sci, "VALUECHANGED_CB", (Icallback)valuechanged_cb);
		IupSetCallback (gapp.sci, "ACTION", (Icallback)action_cb);
	}

	{
		struct
		{
			Ihandle * menu;
			Ihandle * menu1;
			Ihandle * tree1;
			Ihandle * tree2;
			Ihandle * next;
			Ihandle * prop;
			Ihandle * exit;
		} menu;
		menu.tree1 = IupItem ("Tree1", NULL);
		menu.tree2 = IupItem ("Tree2", NULL);
		menu.next = IupItem ("Next", NULL);
		menu.prop = IupItem ("Element properties", NULL);
		menu.exit = IupItem ("Exit", NULL);
		menu.menu1 = IupMenu (menu.tree1, menu.tree2, menu.next, menu.prop, IupSeparator(), menu.exit, NULL);
		menu.menu = IupMenu (IupSubmenu ("Menu", menu.menu1), NULL);
		IupSetCallback (menu.tree1, "ACTION", (Icallback) gapp_view_tree2);
		IupSetCallback (menu.tree2, "ACTION", (Icallback) gapp_view_tree1);
		IupSetCallback (menu.next, "ACTION", (Icallback) btn_next_action);
		IupSetCallback (menu.prop, "ACTION", (Icallback) btn_prop_action);
		IupSetCallback (menu.exit, "ACTION", (Icallback) exit_cb);
		IupSetAttributeHandle(gapp.dlg, "MENU", menu.menu);
		IupSetAttribute (gapp.dlg, "TITLE", "gcovenant");
		IupSetAttribute (gapp.dlg, "RASTERSIZE", "700x500");
		IupSetAttribute (gapp.dlg, "MARGIN", "10x10");
		IupSetAttribute (gapp.dlg, "RASTERSIZE", NULL);
		IupShow (gapp.dlg);
	}

	fstree_build (gapp.tree1, ".", 0);
	fstree_icon (gapp.tree1);
	sci_setup (gapp.sci);

	IupMainLoop();
	IupClose();
	return EXIT_SUCCESS;
}

