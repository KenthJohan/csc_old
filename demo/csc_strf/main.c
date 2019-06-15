#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

#include <csc_strf.h>
#include <csc_tcol.h>

void test_u32 (char const * buf, int base)
{
	printf (TCOL (TCOL_NORMAL, TCOL_MAGENTA, TCOL_WHITE) "%s" TCOL_RST " => ", buf);
	uint32_t num = str_to_u32 (&buf, base);
	printf ("%i '%c'\n", (int) num, *buf);
}

void test_i32 (char const * buf, int base)
{
	printf (TCOL (TCOL_NORMAL, TCOL_MAGENTA, TCOL_WHITE) "%s" TCOL_RST " => ", buf);
	int32_t num = str_to_i32 (&buf, base);
	printf ("%i '%c'\n", (int) num, *buf);
}

void str_to_i32_assert (char const * buf, int base, int32_t ret, char e)
{
	int32_t num = str_to_i32 (&buf, base);
	assert (num == ret);
	assert (*buf == e);
}




int main (int argc, char * argv [])
{
	/*
	test_i32 ("-10101", 2);
	test_i32 ("+10101", 2);
	test_i32 ("-90807", 2);
	test_i32 ("+90807", 2);
	test_i32 ("-7F", 16);
	test_i32 ("+7F", 16);

	str_to_i32_assert ("8", -10, 8, '\0');
	str_to_i32_assert ("45", -10, -35+1, '\0');
	str_to_i32_assert ("123", -10, 83, '\0');
	*/

	//char buf [10];
	//str_from_imax (buf, 10, 146, -3, '.');
	//printf ("Hello: %.*s\n", 10, buf);

	char buf [10];
	strf (buf, 10, "%u32_-2", 44);
	printf ("Hello: %.*s\n", 10, buf);

	return 0;
}
