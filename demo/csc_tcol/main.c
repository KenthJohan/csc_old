#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

#include <csc_tcol.h>

#define MAGENTA TCOL (TCOL_BOLD, TCOL_MAGENTA, TCOL_GREEN)
#define RST TCOL_RST

int main (int argc, char * argv [])
{
	printf (MAGENTA "Hello" RST "\n");
	return 0;
}
