#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <errno.h>


#include "argparse.h"


#define APP_DESCRIPTION0 \
"\nGenerate text."

#define APP_DESCRIPTION1 \
"\nNo additional description of the program is available in this version."

static const char *const usage[] =
{
"xunitxml [options] [[--] args]",
"xunitxml [options]",
NULL,
};

// http://www.concentric.net/~Ttwang/tech/inthash.htm
unsigned long mix(unsigned long a, unsigned long b, unsigned long c)
{
	a=a-b;  a=a-c;  a=a^(c >> 13);
	b=b-c;  b=b-a;  b=b^(a << 8);
	c=c-a;  c=c-b;  c=c^(b >> 13);
	a=a-b;  a=a-c;  a=a^(c >> 12);
	b=b-c;  b=b-a;  b=b^(a << 16);
	c=c-a;  c=c-b;  c=c^(b >> 5);
	a=a-b;  a=a-c;  a=a^(c >> 3);
	b=b-c;  b=b-a;  b=b^(a << 10);
	c=c-a;  c=c-b;  c=c^(b >> 15);
	return c;
}


int probability (double p)
{
	double r = rand();
	return r < (p * ((double)RAND_MAX + 1.0));
}

int main (int argc, char const * argv [])
{
	setbuf (stdout, NULL);
	unsigned long seed = mix(clock(), time(NULL), getpid());
	srand (seed);

	//Define different program options:
	size_t count = 10;
	double p = 0.5;
	char const * filename = NULL;
	struct argparse_option options[] =
	{
		OPT_HELP(),
		OPT_GROUP("Basic options"),
		OPT_INTEGER('n', "count", &count, "Number of chars", NULL, 0, 0),
		OPT_FLOAT('p', "probability", &p, "The probability of assert success", NULL, 0, 0),
		OPT_STRING('f', "filename", &filename, "The filename", NULL, 0, 0),
		OPT_END()
	};

	//Parse program options:
	struct argparse argparse;
	argparse_init (&argparse, options, usage, 0);
	argparse_describe (&argparse, APP_DESCRIPTION0, APP_DESCRIPTION1);
	argc = argparse_parse (&argparse, argc, argv);

	for (size_t i = 0; i < count; ++i)
	{
		if ((i & ((1<<6)-1)) == 0)
		{
			putc ('\n', stdout);
		}
		char c = 'a' + rand () % 26;
		putc (c, stdout);
	}

	assert (rand () & 1);

	return EXIT_SUCCESS;
}







