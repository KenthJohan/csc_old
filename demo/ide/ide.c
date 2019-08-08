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



enum app_scimargin
{
	APP_SCIMARGIN_LINES,
	APP_SCIMARGIN_BM0,
	APP_SCIMARGIN_BM1,
	APP_SCIMARGIN_BM2,
	APP_SCIMARGIN_FOLD,
	APP_SCIMARGIN_TEXT
};

static const char* sampleCode =
{
"/* Block comment */\n"
"#include<stdio.h>\n#include<iup.h>\n\n"
"void SampleTest() {\n  printf(\"Printing float: %f\\n\", 12.5);\n}\n\n"
"void SampleTest2() {\n  printf(\"Printing char: %c\\n\", 'c');\n}\n\n"
"int main(int argc, char **argv) {\n"
"  // Start up IUP\n"
"  IupOpen(&argc, &argv);\n"
"  IupSetGlobal(\"SINGLEINSTANCE\", \"Iup Sample\");\n\n"
"  if(!IupGetGlobal(\"SINGLEINSTANCE\")) {\n"
"    IupClose(); \n"
"    return EXIT_SUCCESS; \n  }\n\n"
"  SampleTest();\n"
"  SampleTest2();\n"
"  printf(\"Printing an integer: %d\\n\", 37);\n\n"
"  IupMainLoop();\n"
"  IupClose();\n"
"  return EXIT_SUCCESS;\n}\n"
};




static Ihandle * gih_sci = NULL;
static Ihandle * gih_tree = NULL;


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

static int valuechanged_cb(Ihandle *self)
{
	printf("VALUECHANGED_CB\n");
	(void)self;
	return IUP_DEFAULT;
}

static int action_cb(Ihandle *self, int insert, int pos, int length, char* text)
{
	printf("ACTION = insert: %d, pos: %d, lenght:%d, text: %s\n", insert, pos, length, text);
	(void)self;
	return IUP_IGNORE;
}


static int btn_open_action (Ihandle* ih)
{
	(void)ih;
	printf ("btn_open_action!\n");
	return IUP_DEFAULT;
}

void IupTextConvertLinColToPosLen(Ihandle* ih, int lin, int col, int *pos, int *len)
{
	IupTextConvertLinColToPos (ih, lin, col, pos);
	char * text = IupGetAttributeId (ih, "LINE", lin);
	//assert (text);
	if (text)
	{
		*len = (int)strlen (text); //Maybe use strnlen?
		//printf ("%d:%d\n", *pos, *len);
	}
	else
	{
		*len = 0;
	}
}

static int btn_next_action (Ihandle* ih)
{
	(void)ih;
	printf ("btn_next_action!\n");
	/*
	static int lin = 0;
	int pos;
	int len;
	IupTextConvertLinColToPosLen (handle_sci, lin, 0, &pos, &len);
	IupSetStrf (handle_sci, "INDICATORCLEARRANGE", "%d:%d", pos, len);
	lin ++;
	IupTextConvertLinColToPosLen (handle_sci, lin, 0, &pos, &len);
	IupSetStrf (handle_sci, "INDICATORFILLRANGE", "%d:%d", pos, len);
	*/
	static int line = 0;
	//IupSetIntId (gih_sci, "MARKERDELETE", line, 8);
	line ++;
	IupSetIntId (gih_sci, "MARKERADD", line, 8);
	IupSetAttribute (gih_sci, "APPEND", "APPEND");
	return IUP_DEFAULT;
}

static int btn_prop_action (Ihandle* ih)
{
	IupShow (IupElementPropertiesDialog (gih_sci));
	(void)ih;
	return IUP_DEFAULT;
}

/*
Walk directory recursive.
Build directory tree in IupTree.
*/
void fstree_build (Ihandle * ih, char * dir0, int id)
{
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
			snprintf (star, MAX_PATH, "%s/%s", dir0, fileinfo.name);
			//printf ("F %x %s\n", fileinfo.attrib, star);
			IupSetAttributeId (ih, "ADDLEAF", id, star);
			if (ext && (strcmp (ext, ".a") == 0))
			{
				IupSetAttributeHandleId (ih, "IMAGE", IupGetInt (ih, "LASTADDNODE"), gih_img_lib);
			}
			else if (ext && (strcmp (ext, ".h") == 0))
			{
				IupSetAttributeHandleId (ih, "IMAGE", IupGetInt (ih, "LASTADDNODE"), gih_img_h);
			}
			else if (ext && (strcmp (ext, ".hpp") == 0))
			{
				IupSetAttributeHandleId (ih, "IMAGE", IupGetInt (ih, "LASTADDNODE"), gih_img_hpp);
			}
			else if (ext && (strcmp (ext, ".c") == 0))
			{
				IupSetAttributeHandleId (ih, "IMAGE", IupGetInt (ih, "LASTADDNODE"), gih_img_c);
			}
			else if (ext && (strcmp (ext, ".cpp") == 0))
			{
				IupSetAttributeHandleId (ih, "IMAGE", IupGetInt (ih, "LASTADDNODE"), gih_img_cpp);
			}
			else if (ext && (strcmp (ext, ".gcov") == 0))
			{
				IupSetAttributeHandleId (ih, "IMAGE", IupGetInt (ih, "LASTADDNODE"), gih_img_gcov);
			}
		}
		else if (fileinfo.attrib & _A_SUBDIR)
		{
			snprintf (star, MAX_PATH, "%s/%s", dir0, fileinfo.name);
			//printf ("D %x %s\n", fileinfo.attrib, star);
			IupSetAttributeId (ih, "ADDBRANCH", id, star);
			fstree_build (ih, star, IupGetInt (ih, "LASTADDNODE"));
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


/*
Update directories and file names for.
Using globals (gih_tree)
*/
void fstree_refresh (Ihandle * ih, int id)
{
	char * dir = IupGetAttributeId (gih_tree, "TITLE", id);
	printf ("list1_refresh %s\n", dir);
	IupSetAttributeId (ih, "DELNODE", id, "CHILDREN");
	fstree_build (ih, dir, id);
}



void fstree_label (Ihandle * ih, char const * str1)
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
void fstree_find_gcov (Ihandle * ih, int id)
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
		if (IupGetAttributeId (ih, "NEXT", i) == NULL) {return;}
		char * ext = strrchr (title, '.');
		if (ext && strcmp (ext, ".gcov") == 0)
		{
			fstree_label (ih, title);
			//printf ("title %i %s\n", i, title);
		}
		i ++;
	}
}


/*
Action generated when a leaf is to be executed.
This action occurs when the user double clicks a leaf, or hits Enter on a leaf.
*/
int iupfs_on_execute (Ihandle *ih, int id)
{
	char const * title = IupGetAttributeId (ih, "TITLE", id);
	if (title == NULL) {return IUP_DEFAULT;}
	char * text = csc_malloc_file (title);
	if (text == NULL) {return IUP_DEFAULT;}
	//printf ("%s\n", text);
	IupSetAttribute(gih_sci, "CLEARALL", NULL);
	IupSetAttribute(gih_sci, "INSERT0", text);
	free (text);
	return IUP_DEFAULT;
}


/*
User left clicks on refresh button from fstree right click menu.
*/
int iupfs_on_refresh (void)
{
	int id = IupGetInt (gih_tree, "VALUE");
	fstree_refresh (gih_tree, id);
	return IUP_DEFAULT;
}

int iupfs_on_gcov (void)
{
	int id = IupGetInt (gih_tree, "VALUE");
	fstree_find_gcov (gih_tree, id);
	return IUP_DEFAULT;
}

int app_seegcov (void)
{
	IupSetAttribute (gih_sci, "APPENDNEWLINE", "No");
	IupSetAttribute (gih_sci, "CLEARALL", NULL);
	int id = IupGetInt (gih_tree, "VALUE");
	char * title = IupGetAttributeId (gih_tree, "TITLE", id);
	char line [2048] = {0};
	printf ("%s\n", title);
	FILE * f = fopen (title, "r");
	if (f == NULL) {return IUP_DEFAULT;}
	while (fgets(line, 2048, f))
	{
		int l = IupGetInt (gih_sci, "LINECOUNT");
		if (CSC_STRNCMP_LITERAL (line, "        -:    0:") == 0) {continue;}
		IupSetAttribute (gih_sci, "APPEND", line+16);
		if (CSC_STRNCMP_LITERAL (line, "    #####") == 0)
		{
			IupSetIntId (gih_sci, "MARKERADD", l-1, 8);
		}
		else if (CSC_STRNCMP_LITERAL (line, "        -:") == 0)
		{

		}
		else
		{
			IupSetIntId (gih_sci, "MARKERADD", l-1, 9);
		}
	}
	fclose (f);

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
		IupSetFunction ("refresh", (Icallback) iupfs_on_refresh);
		IupSetFunction ("gcov", (Icallback) iupfs_on_gcov);
		IupPopup (popup_menu, IUP_MOUSEPOS, IUP_MOUSEPOS);
		IupDestroy (popup_menu);
	}

	if (strcmp (kind, "LEAF") == 0)
	{
		IupSetInt (h, "VALUE", id);
		Ihandle *popup_menu = IupMenu
		(
		IupItem ("update", "update"),
		IupItem ("see gcov", "app_seegcov"),
		NULL
		);
		IupSetFunction ("app_seegcov", (Icallback) app_seegcov);
		IupPopup (popup_menu, IUP_MOUSEPOS, IUP_MOUSEPOS);
		IupDestroy (popup_menu);
	}

	return IUP_DEFAULT;
}



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


int main(int argc, char* argv[])
{
	setbuf (stdout, NULL);
	setbuf (stderr, NULL);
	IupOpen (&argc, &argv);
	ide_images_load ();
	IupSetGlobal ("UTF8MODE", "No");

	{
		Ihandle * btn_open = IupButton ("open", NULL);
		Ihandle * btn_next = IupButton ("next", NULL);
		Ihandle * btn_prop = IupButton ("IupElementPropertiesDialog", NULL);
		IupSetCallback(btn_open, "ACTION", (Icallback)btn_open_action);
		IupSetCallback(btn_next, "ACTION", (Icallback)btn_next_action);
		IupSetCallback(btn_prop, "ACTION", (Icallback)btn_prop_action);
		Ihandle * dlg = IupDialog(IupVbox(btn_open, btn_next, btn_prop, NULL));
		IupSetAttribute(dlg, "TITLE", "IupScintilla");
		IupSetAttribute(dlg, "RASTERSIZE", "700x500");
		IupSetAttribute(dlg, "MARGIN", "10x10");
		IupSetAttribute(dlg, "RASTERSIZE", NULL);
		IupShow(dlg);
	}

	{
		gih_tree = IupTree ();
		IupSetCallback (gih_tree, "EXECUTELEAF_CB", (Icallback)iupfs_on_execute);
		IupSetCallback (gih_tree, "RIGHTCLICK_CB", (Icallback) iupfs_on_rclick);
		int w, h;
		IupGetGlobal_SCREENSIZE (&w, &h);
		//printf ("%i %i\n",w/2,h/2);
		Ihandle * dlg = IupDialog (IupVbox(gih_tree, NULL));
		IupSetStrf (dlg, "RASTERSIZE", "%ix%i", w/2, h/2);
		IupShowXY (dlg, 0, h/2);
		IupSetAttributeId (gih_tree, "TITLE", 0, "..");
		fstree_build (gih_tree, "..", 0);
	}

	{
		IupScintillaOpen();
		gih_sci = IupScintilla();

		//  IupSetAttribute(sci, "VISIBLECOLUMNS", "80");
		//  IupSetAttribute(sci, "VISIBLELINES", "40");
		//IupSetAttribute(sci, "SCROLLBAR", "NO");
		//  IupSetAttribute(sci, "BORDER", "NO");
		IupSetAttribute(gih_sci, "EXPAND", "Yes");
		//  IupSetAttribute(sci, "OVERWRITE", "ON");
		IupSetCallback(gih_sci, "MARGINCLICK_CB", (Icallback)marginclick_cb);
		IupSetCallback(gih_sci, "HOTSPOTCLICK_CB", (Icallback)hotspotclick_cb);
		IupSetCallback(gih_sci, "BUTTON_CB", (Icallback)button_cb);

		//IupSetCallback(handle_sci, "MOTION_CB", (Icallback)motion_cb);
		IupSetCallback(gih_sci, "K_ANY", (Icallback)k_any);
		IupSetCallback(gih_sci, "CARET_CB", (Icallback)caret_cb);
		IupSetCallback(gih_sci, "VALUECHANGED_CB", (Icallback)valuechanged_cb);
		IupSetCallback(gih_sci, "ACTION", (Icallback)action_cb);

		Ihandle *dlg = IupDialog(IupVbox(gih_sci, NULL));
		int w, h;
		IupGetGlobal_SCREENSIZE (&w, &h);
		IupSetAttribute (dlg, "TITLE", "IupScintilla");
		IupSetStrf (dlg, "RASTERSIZE", "%ix%i", w/2, h/2);
		IupSetAttribute (dlg, "MARGIN", "10x10");
		IupShowXY (dlg, w/2, h/2);

		IupSetAttribute(gih_sci, "CLEARALL", "");
		IupSetAttribute(gih_sci, "LEXERLANGUAGE", "cpp");
		IupSetAttribute(gih_sci, "KEYWORDS0", "void struct union enum char short int long double float signed unsigned const static extern auto register volatile bool class private protected public friend inline template virtual asm explicit typename mutable"
		"if else switch case default break goto return for while do continue typedef sizeof NULL new delete throw try catch namespace operator this const_cast static_cast dynamic_cast reinterpret_cast true false using"
		"typeid and and_eq bitand bitor compl not not_eq or or_eq xor xor_eq");
		IupSetAttribute(gih_sci, "STYLEFONT32", "Consolas");
		IupSetAttribute(gih_sci, "STYLEFONTSIZE32", "11");
		IupSetAttribute(gih_sci, "STYLECLEARALL", "Yes");  /* sets all styles to have the same attributes as 32 */
		IupSetAttribute(gih_sci, "STYLEFGCOLOR1", "0 128 0");    // 1-C comment
		IupSetAttribute(gih_sci, "STYLEFGCOLOR2", "0 128 0");    // 2-C++ comment line
		IupSetAttribute(gih_sci, "STYLEFGCOLOR4", "128 0 0");    // 4-Number
		IupSetAttribute(gih_sci, "STYLEFGCOLOR5", "0 0 255");    // 5-Keyword
		IupSetAttribute(gih_sci, "STYLEFGCOLOR6", "160 20 20");  // 6-String
		IupSetAttribute(gih_sci, "STYLEFGCOLOR7", "128 0 0");    // 7-Character
		IupSetAttribute(gih_sci, "STYLEFGCOLOR9", "0 0 255");    // 9-Preprocessor block
		IupSetAttribute(gih_sci, "STYLEFGCOLOR10", "255 0 255"); // 10-Operator
		IupSetAttribute(gih_sci, "STYLEBOLD10", "YES");
		IupSetAttribute(gih_sci, "STYLEHOTSPOT6", "YES");
		IupSetAttribute(gih_sci, "INSERT0", sampleCode);
		IupSetAttribute(gih_sci, "TABSIZE", "4");
		IupSetAttribute(gih_sci, "WHITESPACEVIEW", "VISIBLEALWAYS");
		IupSetAttribute(gih_sci, "WHITESPACEFGCOLOR", "200 200 200");

		IupSetAttributeId (gih_sci, "MARGINWIDTH", APP_SCIMARGIN_LINES, "50");
		IupSetAttributeId (gih_sci, "MARGINWIDTH", APP_SCIMARGIN_BM0, "20");
		IupSetAttributeId (gih_sci, "MARGINWIDTH", APP_SCIMARGIN_BM1, "20");
		IupSetAttributeId (gih_sci, "MARGINWIDTH", APP_SCIMARGIN_BM2, "20");
		IupSetAttributeId (gih_sci, "MARGINWIDTH", APP_SCIMARGIN_FOLD, "20");
		IupSetAttributeId (gih_sci, "MARGINSENSITIVE", APP_SCIMARGIN_LINES, "YES");
		IupSetAttributeId (gih_sci, "MARGINSENSITIVE", APP_SCIMARGIN_BM0, "YES");
		IupSetAttributeId (gih_sci, "MARGINSENSITIVE", APP_SCIMARGIN_BM1, "YES");
		IupSetAttributeId (gih_sci, "MARGINSENSITIVE", APP_SCIMARGIN_BM2, "YES");
		IupSetAttributeId (gih_sci, "MARGINSENSITIVE", APP_SCIMARGIN_FOLD, "YES");

		//https://www.scintilla.org/ScintillaDoc.html#SCI_MARKERDEFINE
		IupSetAttributeId (gih_sci, "MARKERSYMBOL", 4, "CIRCLE");
		IupSetAttributeId (gih_sci, "MARKERSYMBOL", 5, "ROUNDRECT");
		IupSetAttributeId (gih_sci, "MARKERSYMBOL", 6, "ARROW");
		IupSetAttributeId (gih_sci, "MARKERSYMBOL", 7, "SHORTARROW");
		IupSetAttributeId (gih_sci, "MARKERSYMBOL", 8, "SC_MARK_BACKGROUND");

		IupSetIntId(gih_sci, "MARGINMASK", APP_SCIMARGIN_BM0, 1 << 4);
		IupSetIntId(gih_sci, "MARGINMASK", APP_SCIMARGIN_BM1, 1 << 5);
		IupSetIntId(gih_sci, "MARGINMASK", APP_SCIMARGIN_BM2, 1 << 6);
		IupSetIntId(gih_sci, "MARGINMASK", APP_SCIMARGIN_FOLD, 1 << 7);
		IupSetIntId(gih_sci, "MARGINMASK", APP_SCIMARGIN_TEXT, 1 << 8);

		IupSetAttributeId(gih_sci, "MARKERBGCOLOR", 8, "255 0 0");
		IupSetAttributeId(gih_sci, "MARKERALPHA", 8, "80");

		IupSetAttributeId(gih_sci, "MARKERBGCOLOR", 9, "0 255 0");
		IupSetAttributeId(gih_sci, "MARKERALPHA", 9, "80");

		IupSetAttribute(gih_sci, "PROPERTY", "fold=1");
		IupSetAttribute(gih_sci, "PROPERTY", "fold.compact=0");
		IupSetAttribute(gih_sci, "PROPERTY", "fold.comment=1");
		IupSetAttribute(gih_sci, "PROPERTY", "fold.preprocessor=1");

		IupSetAttributeId (gih_sci, "MARGINTYPE", APP_SCIMARGIN_FOLD, "SYMBOL");
		IupSetAttributeId (gih_sci, "MARGINMASKFOLDERS", APP_SCIMARGIN_FOLD, "Yes");

		IupSetAttribute(gih_sci, "MARKERDEFINE", "FOLDER=PLUS");
		IupSetAttribute(gih_sci, "MARKERDEFINE", "FOLDEROPEN=MINUS");
		IupSetAttribute(gih_sci, "MARKERDEFINE", "FOLDEREND=EMPTY");
		IupSetAttribute(gih_sci, "MARKERDEFINE", "FOLDERMIDTAIL=EMPTY");
		IupSetAttribute(gih_sci, "MARKERDEFINE", "FOLDEROPENMID=EMPTY");
		IupSetAttribute(gih_sci, "MARKERDEFINE", "FOLDERSUB=EMPTY");
		IupSetAttribute(gih_sci, "MARKERDEFINE", "FOLDERTAIL=EMPTY");
		IupSetAttribute(gih_sci, "FOLDFLAGS", "LINEAFTER_CONTRACTED");
	}

	IupMainLoop();
	IupClose();
	return EXIT_SUCCESS;
}

