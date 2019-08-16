#pragma once
#include <iup.h>
//#include "Scintilla.h"
#include <iup_scintilla.h>
#include <iup_config.h>
#include <csc_malloc_file.h>
#include <csc_str.h>

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

void sci_setup (Ihandle * gih_sci)
{
	IupSetAttribute(gih_sci, "CLEARALL", "");
	IupSetAttribute(gih_sci, "LEXERLANGUAGE", "cpp");
	IupSetAttribute(gih_sci, "KEYWORDS0", "void struct union enum char short int long double float signed unsigned const static extern auto register volatile bool class private protected public friend inline template virtual asm explicit typename mutable "
	"if else switch case default break goto return for while do continue typedef sizeof NULL new delete throw try catch namespace operator this const_cast static_cast dynamic_cast reinterpret_cast true false using "
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
	IupSetAttributeId(gih_sci, "MARKERALPHA", 8, "25");

	IupSetAttributeId(gih_sci, "MARKERBGCOLOR", 9, "0 255 0");
	IupSetAttributeId(gih_sci, "MARKERALPHA", 9, "25");

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


int sci_gcov_filename (Ihandle * h, char const * filename)
{
	IupSetAttribute (h, "APPENDNEWLINE", "No");
	IupSetAttribute (h, "CLEARALL", NULL);

	IupSetIntId (h, "MARGINWIDTH", 1, 50);
	IupSetAttributeId (h, "MARGINTYPE", 1, "RTEXT");

	FILE * f = fopen (filename, "r");
	if (f == NULL) {return IUP_DEFAULT;}
	char line [2048] = {0};
	while (fgets(line, 2048, f))
	{
		int l = IupGetInt (h, "LINECOUNT") - 1;
		if (CSC_STRNCMP_LITERAL (line, "        -:    0:") == 0) {continue;}
		IupSetAttribute (h, "APPEND", line+16);
		if (CSC_STRNCMP_LITERAL (line, "    #####") == 0)
		{
			IupSetIntId (h, "MARKERADD", l, 8);
		}
		else if (CSC_STRNCMP_LITERAL (line, "        -:") == 0)
		{

		}
		else
		{
			int k;
			char buf [100];
			sscanf (line, "%i", &k);
			snprintf (buf, 100, "%i", k);
			IupSetAttributeId (h, "MARGINTEXT", l, buf);
			IupSetIntId (h, "MARKERADD", l, 9);
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

	IupSetIntId (h, "MARGINWIDTH", 1, 80);
	IupSetAttributeId (h, "MARGINTYPE", 1, "TEXT");
	IupSetAttributeId (h, "MARGINTEXT", 1, "Hej");

	IupSetAttributeId (h, "MARGINTYPE", 2, "TEXT");

	IupSetAttributeId (h, "MARGINTEXT", 4, "Hejdå");


	IupSetAttribute (h, "APPEND", "Banana");


	return IUP_DEFAULT;
}


