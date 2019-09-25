#pragma once
#include <mxml.h>


struct xunit_instance
{
	mxml_node_t * root;
	mxml_node_t * testsuites;
	mxml_node_t * current_testsuite;
};


void xunit_init (struct xunit_instance * item, char const * filename);
void xunit_tofile (struct xunit_instance * item, char const * filename);
void xunit_add_testcase (struct xunit_instance * item, char const * name, char const * failure_msg);
void xunit_set_testsuite (struct xunit_instance * item, char const * name);
