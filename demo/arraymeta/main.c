#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>


#define ARRAY_U8(x,n)\
uint8_t x[(n)+4];\


int main (int argc, char * argv [])
{
	ARRAY_U8(x, 4);


	return 0;
}
