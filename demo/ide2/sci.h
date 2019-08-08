#pragma once
#include <iup.h>
//#include "Scintilla.h"
#include <iup_scintilla.h>

enum app_scimargin
{
	APP_SCIMARGIN_LINES,
	APP_SCIMARGIN_BM0,
	APP_SCIMARGIN_BM1,
	APP_SCIMARGIN_BM2,
	APP_SCIMARGIN_FOLD,
	APP_SCIMARGIN_TEXT
};


void sci_setup1 (Ihandle * gih_sci)
{
	/*
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
	*/
}

void sci_setup (Ihandle * gih_sci)
{
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
	IupSetAttribute(gih_sci, "INSERT0", "default");
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
