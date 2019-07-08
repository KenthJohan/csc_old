#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <ctype.h>

#include <iup.h>
#include <iupcontrols.h>

#include <csc_debug.h>
#include <csc_tcol.h>


enum lex_toktype
{
	LEX_DEFAULT,
	LEX_SPACE,
	LEX_TAB,
	LEX_LF,
	LEX_VOID,
	LEX_MULPTR,
	LEX_CONST,
	LEX_PARENOPEN,
	LEX_PARENCLOSE,
	LEX_BRACOPEN,
	LEX_BRACCLOSE,
	LEX_COMMA,
	LEX_IDENTIFIER,
	LEX_INT,
	LEX_SEMICOLON
};

char const * ast_toktype_tostr (enum lex_toktype t)
{
	switch (t)
	{
	case LEX_DEFAULT: return "AST_DEFAULT";
	case LEX_SPACE: return "AST_SPACE";
	case LEX_TAB: return "AST_TAB";
	case LEX_LF: return "AST_LF";
	case LEX_VOID: return "AST_VOID";
	case LEX_MULPTR: return "AST_MULPTR";
	case LEX_CONST: return "AST_CONST";
	case LEX_PARENOPEN: return "AST_PARENOPEN";
	case LEX_PARENCLOSE: return "AST_PARENCLOSE";
	case LEX_BRACOPEN: return "AST_BRACOPEN";
	case LEX_BRACCLOSE: return "AST_BRACCLOSE";
	case LEX_COMMA: return "AST_COMMA";
	case LEX_IDENTIFIER: return "AST_IDENTIFIER";
	case LEX_INT: return "AST_INT";
	case LEX_SEMICOLON: return "AST_SEMICOLON";
	}
}

void lex_skip (char const ** p, int (*f)(int))
{
	while (f (**p)) {(*p)++;}
}

int lex_isalphadigit (int c)
{
	return isalpha (c) && isdigit (c);
}

int lex_next_strchr (char const ** p, int * line, int * col, char const * str)
{
	int n = 0;
	while (strchr (str, **p))
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
	lex_skip (p, isalpha);
	lex_skip (p, lex_isalphadigit);
	int n = (*p) - q;
	(*col) += n;
	return n;
}

void ast_next (enum lex_toktype * t, char const ** p, char const ** a, char const ** b, int * line, int * col)
{
	(*a) = (*p);
	if (lex_next_strchr (p, line, col, " "))
	{
		(*t) = LEX_SPACE;
	}
	else if (lex_next_strchr (p, line, col, "\n"))
	{
		(*t) = LEX_LF;
	}
	else if (lex_next_cmp (p, col, "void"))
	{
		(*t) = LEX_VOID;
	}
	else if (lex_next_cmp (p, col, "int"))
	{
		(*t) = LEX_INT;
	}
	else if (lex_next_cmp (p, col, "*"))
	{
		(*t) = LEX_MULPTR;
	}
	else if (lex_next_cmp (p, col, "const"))
	{
		(*t) = LEX_CONST;
	}
	else if (lex_next_cmp (p, col, "("))
	{
		(*t) = LEX_PARENOPEN;
	}
	else if (lex_next_cmp (p, col, ")"))
	{
		(*t) = LEX_PARENCLOSE;
	}
	else if (lex_next_cmp (p, col, "{"))
	{
		(*t) = LEX_BRACOPEN;
	}
	else if (lex_next_cmp (p, col, "}"))
	{
		(*t) = LEX_BRACCLOSE;
	}
	else if (lex_next_cmp (p, col, ","))
	{
		(*t) = LEX_COMMA;
	}
	else if (lex_next_cmp (p, col, ";"))
	{
		(*t) = LEX_SEMICOLON;
	}
	else if (lex_next_indentifer (p, col))
	{
		(*t) = LEX_IDENTIFIER;
	}
	else
	{
		(*t) = LEX_DEFAULT;
	}
	(*b) = (*p);
}

void escape (char des [100], const char * a, const char * b)
{
	char * d = des;
	while (1)
	{
		*d = *a;
		if (*a == *b) {break;}
		if (*a == '\n')
		{
			strcpy (d, "<LF>");
			d += 4;
		}
		else
		{
			d ++;
		}
		a ++;
	}
}

void print_token (enum lex_toktype t, char const * code, char const * p, char const * a, char const * b, int line, int col)
{
	//char aa [100] = {0};
	//escape (aa, a, b-1);
	if (t == LEX_LF)
	{
		printf ("%4i %4i %4i:%-4i %10.*s %s", a - code, b - code - 1, line, col, b - a, "", ast_toktype_tostr (t));
	}
	else
	{
		printf ("%4i %4i %4i:%-4i %10.*s %s", a - code, b - code - 1, line, col, b - a, a, ast_toktype_tostr (t));
	}
	//printf ("%4i %4i %4i:%-4i %10s %s", a - code, b - code - 1, line, col, aa, ast_toktype_tostr (t));
	printf ("\n");
}

enum ast_nodetype
{
	AST_START,
	AST_DECLERATION
};

struct ast_node;
struct ast_node
{
	enum ast_nodetype nodetype;
	enum lex_toktype toktype;
	struct ast_node * prev;
	struct ast_node * next;
	struct ast_node * base;
	struct ast_node * branch;
	int i;
};

void ast_insert (struct ast_node * node, struct ast_node * nextnode, enum lex_toktype tok)
{
	if (node->nodetype == AST_START && tok == LEX_VOID)
	{
		nextnode->nodetype = AST_DECLERATION;
		node->branch = nextnode;
		nextnode->base = node;
	}
	else if (node->nodetype == AST_DECLERATION && tok == LEX_MULPTR)
	{
		nextnode->i ++;
		nextnode->nodetype = AST_DECLERATION;
		nextnode->prev = node;
		nextnode->base = node->base;
		node->next = nextnode;
	}
}


void add_node (Ihandle * h, struct ast_node * node, enum lex_toktype t, char const * code, char const * p, char const * a, char const * b, int line, int col)
{
	char text [100] = {0};
	memccpy (text, a, *b, 100);
	if (node->branch)
	{
		IupSetAttribute (h, "ADDBRANCH", text);
	}
	else
	{
		IupSetAttribute (h, "ADDLEAF", text);
	}
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
	enum lex_toktype t = LEX_DEFAULT;
	struct ast_node nodeast = {0};
	nodeast.nodetype = AST_START;


	IupOpen (&argc, &argv);
	Ihandle * tree = IupTree();
	IupSetAttribute(tree, "SHOWRENAME", "YES");
	IupSetAttribute(tree, "TITLE","AST");
	Ihandle* box = IupVbox(IupHbox(tree, IupButton("test", NULL), NULL), NULL);
	Ihandle* dlg = IupDialog(box);
	IupSetAttribute(dlg, "TITLE", "IupTree");
	IupSetAttribute(box, "MARGIN", "200x200");
	IupShowXY (dlg, IUP_CENTER,IUP_CENTER);


	struct ast_node * node = &nodeast;
	for (int i = 0; i < 10; ++i)
	{
		ast_next (&t, &p, &a, &b, &line, &col);
		int pass = (t == LEX_VOID) || (t == LEX_MULPTR);
		if (!pass) {continue;}
		print_token (t, code, p, a, b, line, col);
		struct ast_node * nextnode = calloc (1, sizeof (struct ast_node));
		ast_insert (node, nextnode, t);
		add_node (tree, node, t, code, p, a, b, line, col);
	}


	/*
	IupSetAttribute(tree, "ADDBRANCH","3D");
	IupSetAttribute(tree, "ADDBRANCH","2D");
	IupSetAttribute(tree, "ADDLEAF","test");
	IupSetAttribute(tree, "ADDBRANCH1","parallelogram");
	IupSetAttribute(tree, "ADDLEAF2","diamond");
	IupSetAttribute(tree, "ADDLEAF2","square");
	IupSetAttribute(tree, "ADDBRANCH1","triangle");
	IupSetAttribute(tree, "ADDLEAF2","scalenus");
	IupSetAttribute(tree, "ADDLEAF2","isoceles");
	IupSetAttribute(tree, "ADDLEAF2","equilateral");
	IupSetAttribute(tree, "VALUE","6");
	*/

	IupMainLoop();
	IupClose();

	return EXIT_SUCCESS;
}
