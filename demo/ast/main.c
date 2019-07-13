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

enum tok_type
{
	TOK_VOID = 256,
	TOK_CONST,
	TOK_INT,
	TOK_IDENTIFIER,
	TOK_LITERAL_INTEGER
};

char const * tok_type_tostr (int t)
{
	switch (t)
	{
	case '\0': return "<EOF>";
	case '\r':  return "<CR>";
	case '\n': return "<LF>";
	case '{': return "{";
	case '}': return "}";
	case '(': return "(";
	case ')': return ")";
	case '*': return "*";
	case ':': return ":";
	case ',': return ",";
	case '?': return "?";
	case '+': return "+";
	case '-': return "-";
	case '/': return "/";
	case '<': return "<";
	case '>': return ">";
	case '|': return "|";
	case TOK_VOID: return "AST_VOID";
	case TOK_CONST: return "AST_CONST";
	case TOK_IDENTIFIER: return "AST_IDENTIFIER";
	case TOK_INT: return "AST_INT";
	case TOK_LITERAL_INTEGER: return "TOK_LITERAL_INTEGER";
	}
	return NULL;
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

int lex_isalphadigit (int c)
{
	return isalpha (c) || isdigit (c);
}

void lex_skip (char const ** p, int (*f)(int))
{
	while (f (**p)) {(*p)++;}
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

int lex_next_literal (char const ** p, int * col)
{
	char const * q = (*p);
	lex_skip (p, isdigit);
	int n = (*p) - q;
	(*col) += n;
	return n;
}


int tok_next (char const ** p, int * line, int * col, char const ** a)
{
again:
	(*a) = (*p);
	switch (**p)
	{
	case '\0': return 0;
	case ' ':
		(*col) ++;
		(*p) ++;
		goto again;
	case '\r':
		(*p) ++;
		goto again;
	case '\n':
		(*col) = 0;
		(*line) ++;
		(*p) ++;
		goto again;
	case '{':
	case '}':
	case '(':
	case ')':
	case '*':
	case ':':
	case ',':
	case '?':
	case '+':
	case '-':
	case '/':
	case '<':
	case '>':
		(*p) ++;
		(*col) ++;
		return (*p) [-1];
	}
	if (0) {}
	else if (lex_next_literal (p, col))
	{
		return TOK_LITERAL_INTEGER;
	}
	else if (lex_next_cmp (p, col, "void"))
	{
		return TOK_CONST;
	}
	else if (lex_next_cmp (p, col, "const"))
	{
		return TOK_VOID;
	}
	else if (lex_next_indentifer (p, col))
	{
		return TOK_IDENTIFIER;
	}
	else if (lex_next_indentifer (p, col))
	{
		return TOK_INT;
	}
	return 0;
}

enum ast_nodetype
{
	AST_START,
	AST_DECLERATION,
	AST_FUNCARGUMENTS,
	AST_CALLARGUMENTS,
	AST_IDENTIFIER,
	AST_IDENTIFIER_VARIABLE,
	AST_IDENTIFIER_FUNCTION,
	AST_LITERAL,
	AST_LITERAL_INTEGER,
	AST_LITERAL_STRING,


};

enum ast_action
{
	AST_ADD_BRANCH,
	AST_ADD_LEAF,
	AST_ADD_BASE,
	AST_BRANCH,
	AST_LEAF
};


struct ast_node;
struct ast_node
{
	enum ast_nodetype nodetype;
	int token;
	struct ast_node * prev;
	struct ast_node * next;
	struct ast_node * base;
	struct ast_node * branch;
	char const * a;
	char const * p;
};


/*
2+3*7

e
+2

*/

void ast_insert
(
enum ast_action action,
struct ast_node ** nodep,
struct ast_node * node,
struct ast_node * newnode
)
{
	switch (action)
	{
	case AST_ADD_BRANCH:
		printf ("%1.1s <> %1.1s\n", node->a, newnode->a);
		node->branch = newnode;
		newnode->base = node;
		*nodep = newnode;
		break;
	case AST_ADD_LEAF:
		printf ("%1.1s <> %1.1s\n", node->a, newnode->a);
		node->next = newnode;
		newnode->prev = node;
		newnode->base = node->base;
		*nodep = newnode;
		break;
	case AST_ADD_BASE:
		printf ("%1.1s <> %1.1s\n", node->a, newnode->a);
		newnode->base = node->base;
		newnode->next = node->next;
		newnode->prev = node->prev;
		newnode->branch = node;
		node->base->branch = newnode;
		node->base = newnode;
		*nodep = node;
		break;
	}
}

void ast_insert2 (struct ast_node ** nodep, struct ast_node * node, struct ast_node * newnode)
{
	if (newnode->token == TOK_LITERAL_INTEGER)
	{
		ast_insert (AST_ADD_LEAF, nodep, node, newnode);
	}
	else if (newnode->token == '+')
	{
		ast_insert (AST_ADD_BASE, nodep, node, newnode);
	}
	else if (newnode->token == '*')
	{
		ast_insert (AST_ADD_BASE, nodep, node, newnode);
	}
}


void iup_ast (Ihandle * h, struct ast_node * node, int depth)
{
	char buf [10] = {0};
	if (!node) {return;}
	memccpy (buf, node->a, *node->p, 10);
	if (node->branch)
	{
		printf ("Branch: %04i %1.1s %s\n", depth, buf, tok_type_tostr (node->token));
		IupSetAttributeId (h, "ADDBRANCH", depth, buf);
	}
	else
	{
		printf ("Leaf:   %04i %1.1s %s\n", depth, buf, tok_type_tostr (node->token));
		IupSetAttributeId (h, "ADDLEAF", depth, buf);
	}
	iup_ast (h, node->branch, depth + 1);
	iup_ast (h, node->next, depth);
}


void test (Ihandle * h)
{
	struct ast_node ast = {0};
	struct ast_node * newnode;
	ast.a = "e";
	newnode = calloc (1, sizeof (struct ast_node));
	newnode->a = "+";
	ast.branch = newnode;
	newnode = calloc (1, sizeof (struct ast_node));
	newnode->a = "2";
	ast.branch->branch = newnode;
	newnode = calloc (1, sizeof (struct ast_node));
	newnode->a = "*";
	ast.branch->branch->next = newnode;
	newnode = calloc (1, sizeof (struct ast_node));
	newnode->a = "3";
	ast.branch->branch->next->branch = newnode;
	newnode = calloc (1, sizeof (struct ast_node));
	newnode->a = "7";
	ast.branch->branch->next->branch->next = newnode;
	newnode = calloc (1, sizeof (struct ast_node));
	newnode->a = "6";
	ast.branch->next = newnode;
	iup_ast (h, &ast, 0);
}

int function (Ihandle *ih, int id, int status)
{
	printf("%i\n", id);
	return IUP_DEFAULT;
}

void test2 (struct ast_node * node)
{
	Ihandle * tree = IupTree();
	IupSetAttribute (tree, "SHOWRENAME", "YES");
	IupSetCallback(tree, "SELECTION_CB", (Icallback) function);
	IupSetAttribute (tree, "TITLE","AST");
	Ihandle * dlg = IupDialog(IupVbox(tree, NULL));
	IupShow (dlg);
	iup_ast (tree, node, 0);
}

int main (int argc, char * argv [])
{
	ASSERT (argc);
	ASSERT (argv);
	setbuf (stdout, NULL);
	IupOpen (&argc, &argv);
	Ihandle* box = IupVbox (IupButton("test", NULL), NULL);
	Ihandle* dlg = IupDialog (box);
	IupSetAttribute (dlg, "TITLE", "IupTree");
	IupSetAttribute (box, "MARGIN", "100x100");
	IupShowXY (dlg, IUP_CENTER, IUP_CENTER);

	char const code [] =
	"3+5*";
	char const * p = code;
	char const * a;
	int line = 0;
	int col = 0;
	int tok;
	struct ast_node ast = {0};
	ast.token = 0;
	ast.base = &ast;
	ast.a = code;
	ast.p = code + 4;
	struct ast_node * node = &ast;

	struct ast_node * newnode = calloc (1, sizeof (struct ast_node));
	newnode->token = 0;
	newnode->a = "e";
	newnode->p = newnode->a;
	ast_insert (AST_ADD_BRANCH, &node, node, newnode);

	for (int i = 0; i < 4; ++i)
	{
		tok = tok_next (&p, &line, &col, &a);
		printf ("Token: %s %s\n", tok_type_tostr (tok), a);
		struct ast_node * newnode = calloc (1, sizeof (struct ast_node));
		newnode->token = tok;
		newnode->a = a;
		newnode->p = p-1;
		ast_insert2 (&node, node, newnode);
		test2 (&ast);
	}




	//iup_ast (tree, &ast, 0);

	IupMainLoop();
	IupClose();

	return EXIT_SUCCESS;
}
