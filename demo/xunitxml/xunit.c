#include "xunit.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>

#include <mxml.h>


const char * whitespace_cb (mxml_node_t *node, int where)
{
	const char * element = mxmlGetElement (node);
	//printf ("%s %i\n", element, where);

	if ((strncmp (element, "?xml", 4) == 0) && (where == MXML_WS_AFTER_OPEN))
	{
		return "\n";
	}
	if ((strcmp (element, "testsuites") == 0) && (where == MXML_WS_AFTER_OPEN))
	{
		return "\n";
	}
	if ((strcmp (element, "testsuite") == 0) && (where == MXML_WS_AFTER_OPEN))
	{
		return "\n";
	}
	if ((strcmp (element, "testcase") == 0) && (where == MXML_WS_AFTER_OPEN))
	{
		return "\n";
	}
	if ((strcmp (element, "testcase") == 0) && (where == MXML_WS_AFTER_CLOSE))
	{
		return "\n";
	}
	if ((strcmp (element, "testsuite") == 0) && (where == MXML_WS_BEFORE_OPEN))
	{
		return "\t";
	}
	if ((strcmp (element, "testsuite") == 0) && (where == MXML_WS_BEFORE_CLOSE))
	{
		return "\t";
	}
	if ((strcmp (element, "testsuite") == 0) && (where == MXML_WS_AFTER_CLOSE))
	{
		return "\n";
	}
	if ((strcmp (element, "failure") == 0) && (where == MXML_WS_AFTER_CLOSE))
	{
		return "\n";
	}
	if ((strcmp (element, "testcase") == 0) && (where == MXML_WS_BEFORE_OPEN))
	{
		return "\t\t";
	}
	if ((strcmp (element, "testcase") == 0) && (where == MXML_WS_BEFORE_CLOSE))
	{
		return "\t\t";
	}
	if ((strcmp (element, "failure") == 0) && (where == MXML_WS_BEFORE_OPEN))
	{
		return "\t\t\t";
	}
	if ((strcmp (element, "skipped") == 0) && (where == MXML_WS_BEFORE_OPEN))
	{
		return "\t\t\t";
	}
	if ((strcmp (element, "skipped") == 0) && (where == MXML_WS_AFTER_OPEN))
	{
		return "\n";
	}

	return NULL;
}


void xunit_set_testsuite (struct xunit_instance * item, char const * name)
{
	assert (item->testsuites);
	assert (item->root);
	item->current_testsuite = mxmlFindElement (item->root, item->root, "testsuite", "name", name, MXML_DESCEND);
	if (item->current_testsuite == NULL)
	{
		item->current_testsuite = mxmlNewElement (item->testsuites, "testsuite");
		assert (item->current_testsuite);
		mxmlElementSetAttrf (item->current_testsuite, "name", "%s", name);
	}
	assert (item->current_testsuite);
}


void xunit_add_testcase (struct xunit_instance * item, char const * name, char const * failure_msg)
{
	assert (item->current_testsuite);
	mxml_node_t * testcase = mxmlNewElement (item->current_testsuite, "testcase");
	assert (testcase);
	mxmlElementSetAttrf (testcase, "name", "%s", name);
	if (failure_msg)
	{
		mxml_node_t * failure = mxmlNewElement (testcase, "failure");
		assert (failure);
		mxmlElementSetAttr (failure, "message", "Test failure");
		mxmlNewOpaque (failure, failure_msg);
	}
	else
	{
		//mxml_node_t * skipped = mxmlNewElement (node, "skipped");
	}
}


void xunit_init (struct xunit_instance * item, char const * filename)
{
	mxmlSetWrapMargin (0);
	item->root = NULL;
	item->testsuites = NULL;
	item->current_testsuite = NULL;
	if (filename)
	{
		FILE * fp = fopen (filename, "r");
		assert (fp);
		item->root = mxmlLoadFile (NULL, fp, MXML_OPAQUE_CALLBACK);
		fclose (fp);
		assert (item->root);
		item->testsuites = mxmlFindPath (item->root, "testsuites");
		assert (item->testsuites);
	}
	if (item->root == NULL)
	{
		item->root = mxmlNewXML ("1.0");
	}
	if (item->testsuites == NULL)
	{
		item->testsuites = mxmlNewElement (item->root, "testsuites");
	}
	assert (item->root);
	assert (item->testsuites);
}


void xunit_tofile (struct xunit_instance * item, char const * filename)
{
	FILE * fp = fopen (filename, "w+");
	assert (fp);
	mxmlSaveFile (item->root, fp, MXML_NO_CALLBACK);
	fclose (fp);
}
