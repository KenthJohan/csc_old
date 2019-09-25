#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>

#include <mxml.h>
#include "xunit.h"



int main (int argc, char * argv [])
{
	struct xunit_instance xunit;
	xunit_init (&xunit, "hello.xml");
	//xunit_init (&xunit, NULL);
	xunit_set_testsuite (&xunit, "equality");
	xunit_add_testcase (&xunit, "Check A = B", "Not equal!");
	xunit_tofile (&xunit, "hello1.xml");
	return 0;
}
