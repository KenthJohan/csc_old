#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <ctype.h>

#include <csc_debug.h>
#include <csc_tcol.h>


enum ast_state
{
	AST_ERROR,
	AST_START,
	AST_GLOBAL_IDENTIFIER,
	AST_GLOBAL_PTR,
	AST_GLOBAL_ASSIGN,
	AST_FUNCTION_NAME,
	AST_FARG_IDENTIFIER,
	AST_FARG_PTR,
	AST_FARG_NAME,
	AST_LOCAL_IDENTIFIER
};


char const * ast_state_tostr (enum ast_state state)
{
	switch (state)
	{
	case AST_ERROR: return "AST_ERROR";
	case AST_START: return "AST_START";
	case AST_GLOBAL_IDENTIFIER: return "AST_GLOBAL_IDENTIFIER";
	case AST_GLOBAL_PTR: return "AST_GLOBAL_PTR";
	case AST_GLOBAL_ASSIGN: return "AST_GLOBAL_ASSIGN";
	case AST_FUNCTION_NAME: return "AST_FUNCTION_NAME";
	case AST_FARG_IDENTIFIER: return "AST_FARG_IDENTIFIER";
	case AST_FARG_PTR: return "AST_FARG_PTR";
	case AST_FARG_NAME: return "AST_FARG_NAME";
	case AST_LOCAL_IDENTIFIER: return "AST_LOCAL_IDENTIFIER";
	}
}


struct ast_table
{
	char * a;
	char * b;
};

void ast_skip (char const ** p, int (*f)(int))
{
	while (f (**p)) {(*p)++;}
}

int ast_isalphadigit (int c)
{
	return isalpha (c) && isdigit (c);
}

int ast_next_indentifer (char const ** p)
{
	char const * q;
	q = (*p);
	ast_skip (p, isalpha);
	ast_skip (p, ast_isalphadigit);
	if (q == (*p)) {return 0;}
	return 1;
}

int ast_next_cmp (char const ** p, char const * str)
{
	unsigned l = strlen (str);
	int diff = strncmp (*p, str, l);
	if (diff == 0)
	{
		(*p) += l;
		return l;
	}
	return 0;
}

int ast_peek_cmp (char const * p, char const * str)
{
	ast_skip (&p, isspace);
	unsigned l = strlen (str);
	int diff = strncmp (p, str, l);
	if (diff == 0)
	{
		return l;
	}
	return 0;
}

void ast_next (enum ast_state * state, char const ** p, char const ** a, char const ** b)
{
	switch (*state)
	{
	case AST_START:
	ast_skip (p, isspace);
	*a = *p;
	if (0) {}
	else if (ast_next_indentifer (p))
	{
		(*state) = AST_GLOBAL_IDENTIFIER;
	}
	*b = *p;
	ast_skip (p, isspace);
	break;

	case AST_GLOBAL_PTR:
	case AST_GLOBAL_IDENTIFIER:
	ast_skip (p, isspace);
	*a = *p;
	if (0) {}
	else if (ast_next_cmp (p, "*"))
	{
		(*state) = AST_GLOBAL_PTR;
	}
	else if (ast_next_indentifer (p))
	{
		(*state) = AST_GLOBAL_IDENTIFIER;
		if (ast_peek_cmp (*p, "("))
		{
			(*state) = AST_FUNCTION_NAME;
		}
	}
	else if (ast_next_cmp (p, "="))
	{
		*state = AST_GLOBAL_ASSIGN;
	}
	*b = *p;
	ast_skip (p, isspace);
	break;

	case AST_FUNCTION_NAME:
	ast_skip (p, isspace);
	ast_next_cmp (p, "(");
	*a = *p;
	if (0) {}
	else if (ast_next_indentifer (p))
	{
		(*state) = AST_FARG_IDENTIFIER;
	}
	*b = *p;
	break;


	case AST_FARG_NAME:
	ast_skip (p, isspace);
	if (0) {}
	else if (ast_next_cmp (p, ","))
	{

	}
	else if (ast_next_cmp (p, ")"))
	{
		ast_skip (p, isspace);
		if (ast_next_cmp (p, "{"))
		{
			(*state) = AST_LOCAL_IDENTIFIER;
		}
	}
	case AST_FARG_PTR:
	case AST_FARG_IDENTIFIER:
	ast_skip (p, isspace);
	*a = *p;
	if (0) {}
	else if (ast_next_cmp (p, "*"))
	{
	(*state) = AST_FARG_PTR;
	}
	else if (ast_next_indentifer (p))
	{
	(*state) = AST_FARG_IDENTIFIER;
	if (ast_peek_cmp (*p, ",")||ast_peek_cmp (*p, ")"))
	{
	(*state) = AST_FARG_NAME;
	}
	}
	*b = *p;
	ast_skip (p, isspace);
	break;

	case AST_LOCAL_IDENTIFIER:
	ast_skip (p, isspace);
	*a = *p;
	if (0) {}
	else if (ast_next_indentifer (p))
	{
		(*state) = AST_LOCAL_IDENTIFIER;
	}
	*b = *p;
	ast_skip (p, isspace);
	break;

	}
}

void check (enum ast_state state, char const * code, char const * p, char const * a, char const * b)
{
	printf ("%4i %10.*s  %s\n", p - code, b - a, a, ast_state_tostr (state));
}





int main (int argc, char * argv [])
{
	ASSERT (argc);
	ASSERT (argv);
	setbuf (stdout, NULL);
	char const code [] =
	"void * const foo (int a, int b, int c)\n"
	"{\n"
	"int localvar;"
	"}\n";
	char const * p = code;
	char const * a;
	char const * b;
	uint32_t state = AST_START;
	ast_next (&state, &p, &a, &b);check (state, code, p, a, b);
	ast_next (&state, &p, &a, &b);check (state, code, p, a, b);
	ast_next (&state, &p, &a, &b);check (state, code, p, a, b);
	ast_next (&state, &p, &a, &b);check (state, code, p, a, b);
	ast_next (&state, &p, &a, &b);check (state, code, p, a, b);
	ast_next (&state, &p, &a, &b);check (state, code, p, a, b);
	ast_next (&state, &p, &a, &b);check (state, code, p, a, b);
	ast_next (&state, &p, &a, &b);check (state, code, p, a, b);
	ast_next (&state, &p, &a, &b);check (state, code, p, a, b);
	ast_next (&state, &p, &a, &b);check (state, code, p, a, b);
	ast_next (&state, &p, &a, &b);check (state, code, p, a, b);
	ast_next (&state, &p, &a, &b);check (state, code, p, a, b);
	ast_next (&state, &p, &a, &b);check (state, code, p, a, b);
	return 0;
}
