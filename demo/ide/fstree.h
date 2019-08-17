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


#include <csc_fspath.h>
#include <csc_str.h>
#include <csc_basic.h>
#include <csc_iup.h>

struct fsnode
{
	char path [MAX_PATH];
};


/*
For every userdata (i) that is non null will be free.
*/
void fstree_free_userdata (Ihandle * h)
{
	int i = 0;
	while (1)
	{
		void * title = IupGetAttributeId (h, "TITLE", i);
		if (title == NULL) {break;}
		void * userdata = IupTreeGetUserId (h, i);
		if (userdata)
		{
			free (userdata);
		}
		i ++;
	}
}


/*
Walk directory recursive.
Build directory tree in IupTree.
*/
void fstree_build_recursively (Ihandle * h, char const * dir0, int ref)
{
	//printf ("%i %s\n", id, dir0);
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
			IupSetAttributeId (h, "ADDLEAF", ref, fileinfo.name);
			IupTreeSetUserId (h, IupGetInt(h, "LASTADDNODE"), node);
		}
		else if (fileinfo.attrib & _A_SUBDIR)
		{
			struct fsnode * node = malloc (sizeof (struct fsnode));
			snprintf (star, MAX_PATH, "%s/%s", dir0, fileinfo.name);
			snprintf (node->path, MAX_PATH, "%s/%s", dir0, fileinfo.name);
			//printf ("D %x %s\n", fileinfo.attrib, star);
			IupSetAttributeId (h, "ADDBRANCH", ref, fileinfo.name);
			IupTreeSetUserId (h, IupGetInt(h, "LASTADDNODE"), node);
			fstree_build_recursively (h, star, ref + 1);
		}
		int r = _findnext (handle, &fileinfo);
		if (r != 0)
		{
			break;
		}
	}
	_findclose (handle);
}


void fstree_build (Ihandle * h, char const * dir)
{
	fstree_build_recursively (h, dir, 0);
}


/*
Adds icons from img.h
TODO: Make this more generic.
*/
void fstree_icon (Ihandle * h)
{
	int i = 1;
	while (1)
	{
		//IupSetAttributeId (ih, "USERDATA", i, "Hej");
		char * title = IupGetAttributeId (h, "TITLE", i);
		//struct fsnode * ud = (struct fsnode *) IupGetAttributeId (ih, "USERDATA", i);
		//if (ud == NULL) {break;}
		if (title == NULL) {break;}

		//printf ("%i %s %s\n", i, title, ud->path);

		char * ext = strrchr (title, '.');
		if (ext == NULL) {}
		else if (ext && (strcmp (ext, ".a") == 0))
		{
			IupSetAttributeHandleId (h, "IMAGE", i, gih_img_lib);
		}
		else if (ext && (strcmp (ext, ".h") == 0))
		{
			IupSetAttributeHandleId (h, "IMAGE", i, gih_img_h);
		}
		else if (ext && (strcmp (ext, ".hpp") == 0))
		{
			IupSetAttributeHandleId (h, "IMAGE", i, gih_img_hpp);
		}
		else if (ext && (strcmp (ext, ".c") == 0))
		{
			IupSetAttributeHandleId (h, "IMAGE", i, gih_img_c);
		}
		else if (ext && (strcmp (ext, ".cpp") == 0))
		{
			IupSetAttributeHandleId (h, "IMAGE", i, gih_img_cpp);
		}
		else if (ext && (strcmp (ext, ".gcov") == 0))
		{
			IupSetAttributeHandleId (h, "IMAGE", i, gih_img_gcov);
		}
		i ++;
	}
}


/*
TODO: More generic
*/
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
	//printf ("%s\n", name);

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
Reset color and text.
*/
void fstree_default_style (Ihandle * h)
{
	int i = 0;
	while (1)
	{
		char * title = IupGetAttributeId (h, "TITLE", i);
		if (title == NULL) {break;}
		IupSetAttributeId (h, "COLOR", i, "0 0 0");
		IupSetAttributeId (h, "TITLEFONTSTYLE", i, "Normal");
		//IupSetAttributeId (ih, "NODEVISIBLE", i, "No");
		i ++;
	}
}


/*
Find all gcov files in the IupTree (ih) starting from node (id)
*/
void fstree_gcov_putlabel (Ihandle * h, int id)
{
	int n = IupGetIntId (h, "TOTALCHILDCOUNT", id);
	if (n == 0) {return;}
	// id is a branch.
	// id+1 is the first child of that branch.
	int i = id + 1;
	while (1)
	{
		char * title = IupGetAttributeId (h, "TITLE", i);
		char * kind = IupGetAttributeId (h, "KIND", i);
		char * next = IupGetAttributeId (h, "NEXT", i);
		if (title == NULL) {break;}
		if (kind == NULL) {break;}
		if (strcmp (kind, "BRANCH") == 0)
		{
			i += IupGetIntId (h, "TOTALCHILDCOUNT", i) + 1;
			continue;
		}
		char * ext = strrchr (title, '.');
		if (ext && strcmp (ext, ".gcov") == 0)
		{
			fstree_label_filename (h, title);
			//printf ("title %i %s\n", i, title);
		}
		//printf ("PREV %s %s\n", title, IupGetAttributeId (ih, "PREVIOUS", i));
		if (next == NULL) {break;}
		i ++;
	}
}


/*
Delete all empty directory nodes.
*/
void fstree_remove_empty_dir (Ihandle * h)
{
	int i = 1;
	while (1)
	{
		char * title = IupGetAttributeId (h, "TITLE", i);
		char * kind = IupGetAttributeId (h, "KIND", i);
		if (title == NULL) {break;}
		if (kind == NULL) {break;}
		if ((strcmp (kind, "BRANCH") == 0) && IupTree_nleaf (h, i) == 0)
		{
			IupSetAttributeId (h, "DELNODE", i, "SELECTED");
			continue;
		}
		i ++;
	}
}


/*
Removes every node except extension whitelist (extw).
*/
void fstree_filter_extw (Ihandle * h, char const * extw)
{
	int i = 0;
	while (1)
	{
		char const * title = IupGetAttributeId (h, "TITLE", i);
		char const * kind = IupGetAttributeId (h, "KIND", i);
		if (title == NULL) {break;}
		if (kind == NULL) {break;}
		if (strcmp (kind, "LEAF") == 0)
		{
			char const * ext = strrchr (title, '.');
			if (ext == NULL || csc_str_contains1 (ext, extw, " ") == 0)
			{
				IupSetAttributeId (h, "DELNODE", i, "SELECTED");
				continue;
			}
		}
		i ++;
	}
}


/*
Adds two custom attributes:
FSTREE_ROOT: Walk all files in this directory.
FSTREE_EXTW: Whitelist of file extension. No filtering when NULL.
*/
void fstree_update (Ihandle * h)
{
	//Set to NO to add many items to the tree without updating the display. Default: "YES".
	IupSetAttribute (h, "AUTOREDRAW", "No");
	char const * dir = IupGetAttribute (h, "FSTREE_ROOT");
	if (dir == NULL) {return;}
	IupSetAttribute (h, "TITLE", dir);
	struct fsnode * node = malloc (sizeof (struct fsnode));
	strcpy(node->path, dir);
	fstree_free_userdata (h);
	IupSetAttribute (h, "DELNODE", "CHILDREN");
	IupTreeSetUserId (h, 0, node);
	fstree_build (h, dir);
	char const * extw = IupGetAttribute (h, "FSTREE_EXTW");
	if (extw)
	{
		fstree_filter_extw (h, extw);
	}
	fstree_remove_empty_dir (h);
	fstree_icon (h);
	IupSetAttribute (h, "AUTOREDRAW", "Yes");
}


