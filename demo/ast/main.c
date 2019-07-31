#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <ctype.h>
#include <locale.h>
#include <wchar.h>
#include <fcntl.h>
#include <io.h>

#include <iup.h>
#include <iupcontrols.h>

#include <csc_debug.h>
#include <csc_tcol.h>
#include <csc_basic.h>

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
	case '^': return "^";
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
	case '^':
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


int ast_precedence (int t)
{
	switch (t)
	{
	case '*': return 3;
	case '+': return 2;
	case '^': return 9;
	}
	return 0;
}


struct ast_node;
struct ast_node
{
	enum ast_nodetype nodetype;
	enum ast_action action;
	int token;
	struct ast_node * prev;
	struct ast_node * next;
	struct ast_node * base;
	struct ast_node * branch;
	char const * a;
	char const * p;
};


struct ast_node * ast_insert (enum ast_action action, struct ast_node * node, struct ast_node * newnode)
{
	newnode->action = action;
	switch (action)
	{
	case AST_ADD_BRANCH:
		//printf ("%1.1s <> %1.1s\n", node->a, newnode->a);
		node->branch = newnode;
		newnode->base = node;
		return newnode;
	case AST_ADD_LEAF:
		//printf ("%1.1s <> %1.1s\n", node->a, newnode->a);
		node->next = newnode;
		newnode->prev = node;
		newnode->base = node->base;
		return newnode;
	case AST_ADD_BASE:
		//printf ("%1.1s <> %1.1s\n", node->a, newnode->a);
		newnode->branch = node;
		newnode->base = node->base;
		newnode->next = node->next;
		newnode->prev = node->prev;
		if (node->next) {node->next->prev = newnode;}
		if (node->prev) {node->prev->next = newnode;}
		//if (node->base) {node->base->branch = newnode;}
		node->base = newnode;
		node->next = NULL;
		node->prev = NULL;
		return node;
	}
	return NULL;
}

struct ast_node * ast_insert2 (struct ast_node * node, struct ast_node * newnode)
{
	switch (newnode->token)
	{
	case TOK_LITERAL_INTEGER:
	//if (node->base || node->prev)
	{
		return ast_insert (AST_ADD_LEAF, node, newnode);
	}
	//else
	{
		//return ast_insert (AST_ADD_BRANCH, node, newnode);
	}
	case '-':
	case '+':
	case '*':
	case '^':
	if (node->base)
	{
		int p0;
		int p1;
		p0 = ast_precedence (node->base->token);
		p1 = ast_precedence (newnode->token);
		if (p0 > p1)
		{
			node = node->base;
		}
	}
	return ast_insert (AST_ADD_BASE, node, newnode);
	}
	return NULL;
}

void iup_ast (Ihandle * h, struct ast_node * node, int depth, int leaf)
{
	//Traverse inorder (next, root, branch) because it works well with IupTree.
	if (!node) {return;}
	iup_ast (h, node->next, depth, leaf + 1);
	char buf [40] = {0};
	snprintf (buf, sizeof (buf), "%02i %02i, %2.*s", depth, leaf, node->p - node->a, node->a);
	if (node->branch)
	{
		IupSetAttributeId (h, "ADDBRANCH", depth, buf);
	}
	else
	{
		IupSetAttributeId (h, "ADDLEAF", depth, buf);
	}
	iup_ast (h, node->branch, depth + 1, 0);
}


void ast_print (struct ast_node * node, int depth, int leaf)
{
	//Traverse preorder (root, branch, next).
	if (!node) {return;}
	char buf [40] = {0};
	snprintf (buf, sizeof (buf), "%02i %02i, %2.*s", depth, leaf, node->p - node->a, node->a);
	for (int i = 0; i < depth; i ++)
	{
		printf("| ");
	}
	if (node->branch)
	{

	}
	else
	{
	}
	printf ("%c\n", *node->a);
	ast_print (node->branch, depth + 1, 0);
	ast_print (node->next, depth, leaf + 1);
}


void iup_astidtext (Ihandle * h)
{
	int n = IupGetInt (h, "COUNT");
	for (int i = 0; i < n; ++i)
	{
		char buf [20];
		char * b = IupGetAttributeId (h, "TITLE", i);
		snprintf (buf, 20, "%i: %s", i, b);
		IupSetAttributeId (h, "TITLE", i, buf);
	}
}

struct ast_node * ast_create (char const * name)
{
	struct ast_node * newnode = calloc (1, sizeof (struct ast_node));
	newnode->a = name;
	newnode->p = newnode->a + strlen (name);
	return newnode;
}

int function (Ihandle *ih, int id, int status)
{
	printf("%i\n", id);
	return IUP_DEFAULT;
}

/*
void test ()
{
	struct ast_node * node;
	node = ast_create ("+");
	node->branch = ast_create ("+");
	node->branch->branch = ast_create ("2");
	node->branch->branch->next = ast_create ("*");
	node->branch->branch->next->branch = ast_create ("3");
	node->branch->branch->next->branch->next = ast_create ("7");
	node->branch->next = ast_create ("6");
	Ihandle * tree = IupTree();
	IupSetAttribute (tree, "SHOWRENAME", "YES");
	IupSetCallback(tree, "SELECTION_CB", (Icallback) function);
	IupSetAttribute (tree, "TITLE","AST");
	Ihandle * dlg = IupDialog(IupVbox(tree, NULL));
	IupShow (dlg);
	iup_ast (tree, node, 0, 0, 0);
}

void test3 ()
{
	struct ast_node * ast = ast_create ("e");
	ast->base = ast;
	struct ast_node * node = ast;
	node = ast_insert (AST_ADD_BRANCH, node, ast_create ("3"));
	node = ast_insert (AST_ADD_BASE, node, ast_create ("+"));
	node = ast_insert (AST_ADD_LEAF, node, ast_create ("4"));
	node = ast_insert (AST_ADD_BASE, node, ast_create ("*"));
	Ihandle * tree = IupTree ();
	IupSetAttribute (tree, "SHOWRENAME", "YES");
	IupSetCallback (tree, "SELECTION_CB", (Icallback) function);
	IupSetAttribute (tree, "TITLE","AST");
	Ihandle * dlg = IupDialog (IupVbox(tree, NULL));
	IupShow (dlg);
	iup_ast (tree, ast, 0, 0, 0);
}
*/

void showast (struct ast_node * node)
{
	Ihandle * tree = IupTree();
	IupSetAttribute (tree, "SHOWRENAME", "YES");
	IupSetCallback(tree, "SELECTION_CB", (Icallback) function);
	IupSetAttribute (tree, "TITLE","AST");

	IupSetAttribute (tree, "FONT","Courier, 14");

	Ihandle * dlg = IupDialog(IupVbox(tree, NULL));
	IupShow (dlg);
	iup_ast (tree, node, 0, 0);
	//iup_astidtext (tree);
}

int main (int argc, char * argv [])
{
	ASSERT (argc);
	ASSERT (argv);
	setbuf (stdout, NULL);
	IupOpen (&argc, &argv);
	/*
	Ihandle* box = IupVbox (IupButton("test", NULL), NULL);
	Ihandle* dlg = IupDialog (box);
	IupSetAttribute (dlg, "TITLE", "IupTree");
	IupSetAttribute (box, "MARGIN", "100x100");
	IupShowXY (dlg, IUP_CENTER, IUP_CENTER);
	*/


	char const code [] =
	"2 ^ 3 * 4 + 5 ^ 6 * 7";
	char const * p = code;
	char const * a;
	int line = 0;
	int col = 0;
	int tok;
	struct ast_node * ast = ast_create (code);
	struct ast_node * node = ast;
	node = ast_insert (AST_ADD_BRANCH, node, ast_create ("E"));
	//node = ast_insert (AST_ADD_BRANCH, node, ast_create ("E"));
	for (int i = 0; i < 11; ++i)
	{
		tok = tok_next (&p, &line, &col, &a);
		printf ("Token: %20s %4.*s\n", tok_type_tostr (tok), p-a, a);
		struct ast_node * newnode = calloc (1, sizeof (struct ast_node));
		newnode->token = tok;
		newnode->a = a;
		newnode->p = p;
		node = ast_insert2 (node, newnode);
	}

	ast_print (ast, 0, 0);
	showast (ast);


	//test3 ();

	IupMainLoop ();
	IupClose ();

	return EXIT_SUCCESS;
}
