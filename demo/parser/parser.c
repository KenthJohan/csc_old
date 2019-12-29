#include <assert.h>
#include <stdio.h>

#include <csc_debug.h>
#include <csc_malloc_file.h>



enum token
{
	TOKEN_COMMA = ',',
	TOKEN_PARENTHESIS_LEFT = '(',
	TOKEN_PARENTHESIS_RIGHT = ')',
	TOKEN_BRACKET_CURLY_LEFT = '{',
	TOKEN_BRACKET_CURLY_RIGHT = '}',
	TOKEN_INT,
	TOKEN_IDENTIFIER,
};

struct node4
{
	struct node4 * l;
	struct node4 * r;
	struct node4 * p;
};



void parse ()
{

}


int main (int argc, char * argv [])
{
	ASSERT (argc);
	ASSERT (argv);

	char * text = csc_malloc_file ("../parser/test.c");
	ASSERT (text);

	printf ("%s\n", text);

	free (text);
	text = NULL;

	return 0;
}
