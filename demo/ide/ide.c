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
	IupSetIntId (gih_sci, "MARKERDELETE", line, 8);
	line ++;
	IupSetIntId (gih_sci, "MARKERADD", line, 8);
	return IUP_DEFAULT;
}

static int btn_prop_action (Ihandle* ih)
{
	IupShow (IupElementPropertiesDialog (gih_sci));
	(void)ih;
	return IUP_DEFAULT;
}


void list1 (Ihandle * ih, char * dir0)
{
	struct _finddata_t fileinfo;
	int i = IupGetInt (ih, "LASTADDNODE");
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
		if(strcmp(fileinfo.name, ".") == 0 || strcmp(fileinfo.name, "..") == 0){}
		else if (fpath_hidden2 (fileinfo.name)){}
		else if ((fileinfo.attrib & _A_SUBDIR) == 0)
		{
			snprintf (star, MAX_PATH, "%s/%s", dir0, fileinfo.name);
			//printf ("F %x %s\n", fileinfo.attrib, star);
			IupSetAttributeId (ih, "ADDLEAF", i, star);
			char * ext = strrchr (fileinfo.name, '.');
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
		}
		else if (fileinfo.attrib & _A_SUBDIR)
		{
			snprintf (star, MAX_PATH, "%s/%s", dir0, fileinfo.name);
			//printf ("D %x %s\n", fileinfo.attrib, star);
			IupSetAttributeId (ih, "ADDBRANCH", i, star);
			list1 (ih, star);
		}
		int r = _findnext (handle, &fileinfo);
		if (r != 0)
		{
			break;
		}
	}
	_findclose(handle);
}





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










int main(int argc, char* argv[])
{
	setbuf (stdout, NULL);
	setbuf (stderr, NULL);
	IupOpen (&argc, &argv);
	ide_images_load ();

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
		IupSetCallback(gih_tree, "EXECUTELEAF_CB", (Icallback)iupfs_on_execute);
		IupShow(IupDialog(IupVbox(gih_tree, NULL)));
		IupSetAttributeId (gih_tree, "TITLE", 0, "..");
		list1 (gih_tree, "..");
	}

	{
		IupScintillaOpen();
		IupSetGlobal("UTF8MODE", "No");
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
		IupSetAttribute(dlg, "TITLE", "IupScintilla");
		IupSetAttribute(dlg, "RASTERSIZE", "700x500");
		IupSetAttribute(dlg, "MARGIN", "10x10");
		IupSetAttribute(dlg, "RASTERSIZE", NULL);
		IupShow(dlg);

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

