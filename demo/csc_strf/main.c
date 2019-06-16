#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

#include <csc_strf.h>
#include <csc_tcol.h>

void test_u32 (char const * buf, int8_t base)
{
	printf (TCOL (TCOL_NORMAL, TCOL_MAGENTA, TCOL_WHITE) "%s" TCOL_RST " => ", buf);
	uint32_t num = str_to_u32 (&buf, base);
	printf ("%i '%c'\n", (int) num, *buf);
}

void test_i32 (char const * buf, int8_t base)
{
	printf (TCOL (TCOL_NORMAL, TCOL_MAGENTA, TCOL_WHITE) "%s" TCOL_RST " => ", buf);
	int32_t num = str_to_i32 (&buf, base);
	printf ("%i '%c'\n", (int) num, *buf);
}

void str_to_i32_assert (char const * buf, int8_t base, int32_t ret, char e)
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

	/*
	char buf [10];

	strf (buf, 10, "%i8_10", 44);printf ("Hello: %.*s\n", 10, buf);
	strf (buf, 10, "%i16_10", 44);printf ("Hello: %.*s\n", 10, buf);
	strf (buf, 10, "%i32_10", 44);printf ("Hello: %.*s\n", 10, buf);
	strf (buf, 10, "%i8_10", -44);printf ("Hello: %.*s\n", 10, buf);
	strf (buf, 10, "%i16_10", -44);printf ("Hello: %.*s\n", 10, buf);
	strf (buf, 10, "%i32_10", -44);printf ("Hello: %.*s\n", 10, buf);
	strf (buf, 10, "%u8_10", 44);printf ("Hello: %.*s\n", 10, buf);
	strf (buf, 10, "%u16_10", 44);printf ("Hello: %.*s\n", 10, buf);
	strf (buf, 10, "%u32_10", 44);printf ("Hello: %.*s\n", 10, buf);

	strf (buf, 10, "%i8_16", 44);printf ("Hello: %.*s\n", 10, buf);
	strf (buf, 10, "%i16_16", 44);printf ("Hello: %.*s\n", 10, buf);
	strf (buf, 10, "%i32_16", 44);printf ("Hello: %.*s\n", 10, buf);
	strf (buf, 10, "%u8_16", 44);printf ("Hello: %.*s\n", 10, buf);
	strf (buf, 10, "%u16_16", 44);printf ("Hello: %.*s\n", 10, buf);
	strf (buf, 10, "%u32_16", 44);printf ("Hello: %.*s\n", 10, buf);

	strf (buf, 10, "%i8_-16", 44);printf ("Hello: %.*s\n", 10, buf);
	strf (buf, 10, "%i16_-16", 44);printf ("Hello: %.*s\n", 10, buf);
	strf (buf, 10, "%i32_-16", 44);printf ("Hello: %.*s\n", 10, buf);
	strf (buf, 10, "%u8_-16", 44);printf ("Hello: %.*s\n", 10, buf);
	strf (buf, 10, "%u16_-16", 44);printf ("Hello: %.*s\n", 10, buf);
	strf (buf, 10, "%u32_-16, %u16_-16", 44, 44);printf ("Hello: %.*s\n", 10, buf);
	*/

	/*
	int64_t a = -1000;
	uint64_t b = a;
	int64_t c = b;
	printf ("%i %u\n", (uint64_t)c, (uint64_t)c);
	printf ("%i %u\n", (uint64_t)b, (uint64_t)b);
*/
	//uint32_t p = 0;
	//p |= STR_PBASE (-10);
	//printf ("Hello: %x %i\n", p, (int8_t)STR_UBASE (p));

	str_printf ("%i32 : %i16_2 %u8_10", -32, -4, 255);

	/*
	char buf [] = "Hello";
	str_rev (buf, sizeof (buf)-1);
	printf ("%s\n", buf);
	*/

	/*
	int a = 2;
	int b = 3;
	printf ("%i %i\n", a, b);
	STR_SWAP (int, a, b);
	printf ("%i %i\n", a, b);
	*/

	return 0;
}
