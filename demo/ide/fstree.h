#pragma once
#include <io.h>
#include <iup.h>
#include <io.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <stdio.h>
#include <time.h>
#include <uv.h>

#include "img.h"

struct fsnode
{
	char path [MAX_PATH];
};


void fstree_free_userdata (Ihandle * ih)
{
	int i = 0;
	while (1)
	{
		void * title = IupGetAttributeId (ih, "USERDATA", i);
		if (title == NULL) {break;}
		void * userdata = IupGetAttributeId (ih, "USERDATA", i);
		if (userdata == NULL) {continue;}
		free (userdata);
	}
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
		if(strcmp(fileinfo.name, ".") == 0 || strcmp(fileinfo.name, "..") == 0){}
		else if (csc_fspath_ishidden (fileinfo.name)){}
		else if ((fileinfo.attrib & _A_SUBDIR) == 0)
		{
			struct fsnode * node = malloc (sizeof (struct fsnode));
			snprintf (star, MAX_PATH, "%s/%s", dir0, fileinfo.name);
			snprintf (node->path, MAX_PATH, "%s/%s", dir0, fileinfo.name);
			//printf ("F %x %s\n", fileinfo.attrib, star);
			IupSetAttributeId (ih, "ADDLEAF", ref, fileinfo.name);
			IupSetAttributeId (ih, "USERDATA", IupGetInt(ih, "LASTADDNODE"), node->path);
		}
		else if (fileinfo.attrib & _A_SUBDIR)
		{
			struct fsnode * node = malloc (sizeof (struct fsnode));
			snprintf (star, MAX_PATH, "%s/%s", dir0, fileinfo.name);
			snprintf (node->path, MAX_PATH, "%s/%s", dir0, fileinfo.name);
			//printf ("D %x %s\n", fileinfo.attrib, star);
			IupSetAttributeId (ih, "ADDBRANCH", ref, fileinfo.name);
			IupSetAttributeId (ih, "USERDATA", IupGetInt(ih, "LASTADDNODE"), (void*)node);
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
	int i = 1;
	while (1)
	{
		//IupSetAttributeId (ih, "USERDATA", i, "Hej");
		char * title = IupGetAttributeId (ih, "TITLE", i);
		//struct fsnode * ud = (struct fsnode *) IupGetAttributeId (ih, "USERDATA", i);
		//if (ud == NULL) {break;}
		if (title == NULL) {break;}

		//printf ("%i %s %s\n", i, title, ud->path);

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


/*
Find all gcov files in the IupTree (ih) starting from node (id)
*/
void fstree_copy (Ihandle * src, Ihandle * des, char const * extfilter)
{
	int i;

	i = 1;
	while (1)
	{
		char * title = IupGetAttributeId (src, "TITLE", i);
		char * kind = IupGetAttributeId (src, "KIND", i);
		int depth = IupGetIntId (src, "DEPTH", i);
		if (title == NULL) {break;}
		if (kind == NULL) {break;}

		if (depth == 0) {}
		else if (strcmp (kind, "LEAF") == 0)
		{
			char * ext = strrchr (title, '.');
			if ((extfilter == NULL) || (ext && csc_str_contains1 (ext, extfilter, " ")))
			{
				//printf ("ADDLEAF %i %s\n", depth-1, title);
				IupSetAttributeId (des, "ADDLEAF", depth-1, title);
				IupSetAttributeId (des, "USERDATA", IupGetInt (des, "LASTADDNODE"), (int)i);
			}
		}
		else if (strcmp (kind, "BRANCH") == 0)
		{
			//printf ("ADDBRANCH %i %s\n", depth-1, title);
			IupSetAttributeId (des, "ADDBRANCH", depth-1, title);
			IupSetAttributeId (des, "USERDATA", IupGetInt (des, "LASTADDNODE"), (int)i);
		}
		i ++;
	}

	i = 1;
	while (1)
	{
		char * title = IupGetAttributeId (des, "TITLE", i);
		char * kind = IupGetAttributeId (des, "KIND", i);
		if (title == NULL) {break;}
		if (kind == NULL) {break;}
		if ((strcmp (kind, "BRANCH") == 0) && IupTree_nleaf (des, i) == 0)
		{
			IupSetAttributeId (des, "DELNODE", i, "SELECTED");
			continue;
		}
		i ++;
	}
}









