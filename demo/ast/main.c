#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <ctype.h>

#include <csc_debug.h>
#include <csc_tcol.h>


enum ast_toktype
{
	AST_NORMAL,
	AST_WHITESPACE,
	AST_VOID,
	AST_PTR,
	AST_CONST,
	AST_PARENOPEN,
	AST_PARENCLOSE,
	AST_BRACOPEN,
	AST_BRACCLOSE,
	AST_COMMA,
	AST_IDENTIFIER
};

enum ast_action
{
	AST_NOTHING,
	AST_BRANCH_OPEN,
	AST_BRANCH_COMPLETE,
	AST_GROW
};


char const * ast_toktype_tostr (enum ast_toktype t)
{
	switch (t)
	{
	case AST_NORMAL: return "AST_NORMAL";
	case AST_WHITESPACE: return "AST_WHITESPACE";
	case AST_VOID: return "AST_VOID";
	case AST_PTR: return "AST_PTR";
	case AST_CONST: return "AST_CONST";
	case AST_PARENOPEN: return "AST_PARENOPEN";
	case AST_PARENCLOSE: return "AST_PARENCLOSE";
	case AST_BRACOPEN: return "AST_BRACOPEN";
	case AST_BRACCLOSE: return "AST_BRACCLOSE";
	case AST_COMMA: return "AST_COMMA";
	case AST_IDENTIFIER: return "AST_IDENTIFIER";
	}
}


void ast_skip (char const ** p, int (*f)(int))
{
	while (f (**p)) {(*p)++;}
}

int ast_isalphadigit (int c)
{
	return isalpha (c) && isdigit (c);
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

int ast_next_cb (char const ** p, int (*f)(int))
{
	if (!f (**p)) {return 0;}
	while (f (**p)){(*p)++;}
	return 1;
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

int ast_cmpn (char const * p, char const * str)
{
	if ((*p) == '\0') {return 0;}
	unsigned l = strlen (str);
	int diff = strncmp (p, str, l);
	if (diff == 0)
	{
		return l;
	}
	return 0;
}

int lex_next_wp (char const ** p, int * line, int * col)
{
	int n = 0;
	while (isspace (**p))
	{
		if ((**p) == '\n')
		{
			(*col) = 0;
			(*line) ++;
		}
		else
		{
			(*col) ++;
		}
		n ++;
		(*p) ++;
	}
	return n;
}

int lex_next_cmp (char const ** p, int * col, char const * str)
{
	unsigned l = strlen (str);
	int diff = strncmp (*p, str, l);
	if (diff == 0)
	{
		(*p) += l;
		(*col) += l;
		return l;
	}
	return 0;
}

int lex_next_indentifer (char const ** p, int * col)
{
	char const * q = (*p);
	ast_skip (p, isalpha);
	ast_skip (p, ast_isalphadigit);
	int n = (*p) - q;
	(*col) += n;
	return n;
}

void ast_next (enum ast_toktype * t, char const ** p, char const ** a, char const ** b, int * line, int * col)
{
	(*a) = (*p);
	if (lex_next_wp (p, line, col))
	{
		(*t) = AST_WHITESPACE;
	}
	else if (lex_next_cmp (p, col, "void"))
	{
		(*t) = AST_VOID;
	}
	else if (lex_next_cmp (p, col, "*"))
	{
		(*t) = AST_PTR;
	}
	else if (lex_next_cmp (p, col, "const"))
	{
		(*t) = AST_CONST;
	}
	else if (lex_next_cmp (p, col, "("))
	{
		(*t) = AST_PARENOPEN;
	}
	else if (lex_next_cmp (p, col, ")"))
	{
		(*t) = AST_PARENCLOSE;
	}
	else if (lex_next_cmp (p, col, "{"))
	{
		(*t) = AST_BRACOPEN;
	}
	else if (lex_next_cmp (p, col, "}"))
	{
		(*t) = AST_BRACCLOSE;
	}
	else if (lex_next_cmp (p, col, ","))
	{
		(*t) = AST_COMMA;
	}
	else if (lex_next_indentifer (p, col))
	{
		(*t) = AST_IDENTIFIER;
	}
	else
	{
		(*t) = AST_NORMAL;
	}
	(*b) = (*p);
}


void check (enum ast_toktype t, char const * code, char const * p, char const * a, char const * b, int line, int col)
{
	printf ("%4i %4i %4i:%-4i %10.*s %s", a - code, b - code - 1, line, col, b - a, a, ast_toktype_tostr (t));
	printf ("\n");
}



int main (int argc, char * argv [])
{
	ASSERT (argc);
	ASSERT (argv);
	setbuf (stdout, NULL);
	char const code [] =
	"void *    const foo (int a, int b, int c)\n"
	"{\n"
	"int localvar;"
	"}\n";
	char const * p = code;
	char const * a;
	char const * b;
	int line = 0;
	int col = 0;

	enum ast_toktype t = AST_NORMAL;
	ast_next (&t, &p, &a, &b, &line, &col);check (t, code, p, a, b, line, col);
	ast_next (&t, &p, &a, &b, &line, &col);check (t, code, p, a, b, line, col);
	ast_next (&t, &p, &a, &b, &line, &col);check (t, code, p, a, b, line, col);
	ast_next (&t, &p, &a, &b, &line, &col);check (t, code, p, a, b, line, col);
	ast_next (&t, &p, &a, &b, &line, &col);check (t, code, p, a, b, line, col);
	ast_next (&t, &p, &a, &b, &line, &col);check (t, code, p, a, b, line, col);
	ast_next (&t, &p, &a, &b, &line, &col);check (t, code, p, a, b, line, col);
	ast_next (&t, &p, &a, &b, &line, &col);check (t, code, p, a, b, line, col);
	ast_next (&t, &p, &a, &b, &line, &col);check (t, code, p, a, b, line, col);
	ast_next (&t, &p, &a, &b, &line, &col);check (t, code, p, a, b, line, col);
	ast_next (&t, &p, &a, &b, &line, &col);check (t, code, p, a, b, line, col);
	ast_next (&t, &p, &a, &b, &line, &col);check (t, code, p, a, b, line, col);
	ast_next (&t, &p, &a, &b, &line, &col);check (t, code, p, a, b, line, col);
	ast_next (&t, &p, &a, &b, &line, &col);check (t, code, p, a, b, line, col);
	ast_next (&t, &p, &a, &b, &line, &col);check (t, code, p, a, b, line, col);
	ast_next (&t, &p, &a, &b, &line, &col);check (t, code, p, a, b, line, col);
	ast_next (&t, &p, &a, &b, &line, &col);check (t, code, p, a, b, line, col);
	ast_next (&t, &p, &a, &b, &line, &col);check (t, code, p, a, b, line, col);
	ast_next (&t, &p, &a, &b, &line, &col);check (t, code, p, a, b, line, col);
	ast_next (&t, &p, &a, &b, &line, &col);check (t, code, p, a, b, line, col);
	ast_next (&t, &p, &a, &b, &line, &col);check (t, code, p, a, b, line, col);
	ast_next (&t, &p, &a, &b, &line, &col);check (t, code, p, a, b, line, col);
	ast_next (&t, &p, &a, &b, &line, &col);check (t, code, p, a, b, line, col);
	ast_next (&t, &p, &a, &b, &line, &col);check (t, code, p, a, b, line, col);
	ast_next (&t, &p, &a, &b, &line, &col);check (t, code, p, a, b, line, col);
	ast_next (&t, &p, &a, &b, &line, &col);check (t, code, p, a, b, line, col);
	ast_next (&t, &p, &a, &b, &line, &col);check (t, code, p, a, b, line, col);
	ast_next (&t, &p, &a, &b, &line, &col);check (t, code, p, a, b, line, col);
	ast_next (&t, &p, &a, &b, &line, &col);check (t, code, p, a, b, line, col);
	ast_next (&t, &p, &a, &b, &line, &col);check (t, code, p, a, b, line, col);
	ast_next (&t, &p, &a, &b, &line, &col);check (t, code, p, a, b, line, col);
	ast_next (&t, &p, &a, &b, &line, &col);check (t, code, p, a, b, line, col);
	ast_next (&t, &p, &a, &b, &line, &col);check (t, code, p, a, b, line, col);
	return 0;
}
