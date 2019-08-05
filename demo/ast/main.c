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
#include <stddef.h>

#include <iup.h>
#include <iupcontrols.h>

#include <csc_debug.h>
#include <csc_tcol.h>
#include <csc_basic.h>

#define container_of(ptr, type, member) ((type *)(void *)((char *)(ptr) - offsetof(type, member)))

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
	size_t l = strlen (str);
	int diff = strncmp (*p, str, l);
	if (diff == 0)
	{
		(*p) += l;
		(*col) += l;
		return (int) l;
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
	ptrdiff_t n = (*p) - q;
	(*col) += n;
	return (int)n;
}

int lex_next_literal (char const ** p, int * col)
{
	char const * q = (*p);
	lex_skip (p, isdigit);
	ptrdiff_t n = (*p) - q;
	(*col) += n;
	return (int)n;
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


int ast_precedence (int t)
{
	switch (t)
	{
	case '*': return 3;
	case '+': return 2;
	case '^': return 9;
	//case TOK_IDENTIFIER: return 100;
	//case '(': return 100;
	}
	return 0;
}


//https://en.wikipedia.org/wiki/Left-child_right-sibling_binary_tree
struct bitree2;
struct bitree2
{
	struct bitree2 * prev;
	struct bitree2 * next;
	struct bitree2 * parent;
	struct bitree2 * child;
};

void bitree2_add_child (struct bitree2 * parent, struct bitree2 * child)
{
	parent->child = child;
	child->parent = parent;
}

void bitree2_add_sibling (struct bitree2 * sibling0, struct bitree2 * sibling1)
{
	sibling0->next = sibling1;
	sibling1->prev = sibling0;
	sibling1->parent = sibling0->parent;
}

void bitree2_add_parent (struct bitree2 * node, struct bitree2 * newnode)
{
	newnode->child = node;
	newnode->parent = node->parent;
	newnode->next = node->next;
	newnode->prev = node->prev;
	if (node->next) {node->next->prev = newnode;}
	if (node->prev) {node->prev->next = newnode;}
	if (node->parent && (node->parent->child == node)) {node->parent->child = newnode;}
	node->parent = newnode;
	node->next = NULL;
	node->prev = NULL;
}



struct ast_node;
struct ast_node
{
	enum ast_nodetype kind;
	int token;
	char const * a;
	char const * p;
	struct bitree2 tree;
};


void ast_add (struct ast_node * node, char const * code)
{
	char const * p = code;
	char const * a;
	int line = 0;
	int col = 0;
	int tok;
	struct ast_node * newnode = NULL;

	tok = tok_next (&p, &line, &col, &a);
	if (tok == 0) {return;}

	while (1)
	{
		//printf ("Token: %20s %4.*s\n", tok_type_tostr (tok), p-a, a);
		switch (tok)
		{
		case TOK_LITERAL_INTEGER:
		case TOK_IDENTIFIER:
			newnode = calloc (1, sizeof (struct ast_node));
			newnode->token = tok;
			newnode->a = a;
			newnode->p = p;
			if (node->kind == AST_IDENTIFIER_FUNCTION)
			{
				bitree2_add_child (&(node->tree), &(newnode->tree));
			}
			else
			{
				bitree2_add_sibling (&(node->tree), &(newnode->tree));
			}
			node = newnode;
			tok = tok_next (&p, &line, &col, &a);
			if (tok == 0) {return;}
			break;

		case '-':
		case '+':
		case '*':
		case '^':
			if (node->tree.parent)
			{
				struct ast_node * parent = container_of (node->tree.parent, struct ast_node, tree);
				int p0 = ast_precedence (parent->token);
				int p1 = ast_precedence (tok);
				if (p0 > p1)
				{
					//bitree2_add_parent (node->tree.parent, &(newnode->tree));
					node = parent;
					break;
				}
			}
			newnode = calloc (1, sizeof (struct ast_node));
			newnode->token = tok;
			newnode->a = a;
			newnode->p = p;
			bitree2_add_parent (&(node->tree), &(newnode->tree));
			tok = tok_next (&p, &line, &col, &a);
			if (tok == 0) {return;}
			break;


		case '(':
			if (node->token == TOK_IDENTIFIER)
			{
				node->kind = AST_IDENTIFIER_FUNCTION;
			}
			break;

		case ')':
			node = container_of (node->tree.parent, struct ast_node, tree);
			break;


		}// END switch
	}// END while
}


struct ast_node * ast_add_child (struct ast_node * node, struct ast_node * newnode)
{
	bitree2_add_child (&(node->tree), &(newnode->tree));
	return newnode;
}



void ast_iuptree (Ihandle * h, struct bitree2 * node, int depth, int leaf)
{
	//Traverse inorder (next, root, child) because it works well with IupTree.
	if (!node) {return;}
	ast_iuptree (h, node->next, depth, leaf + 1);
	char buf [40] = {0};
	struct ast_node * n = container_of (node, struct ast_node, tree);
	snprintf (buf, sizeof (buf), "%02i %02i, %2.*s", depth, leaf, n->p - n->a, n->a);
	if (node->child)
	{
		IupSetAttributeId (h, "ADDBRANCH", depth, buf);
	}
	else
	{
		IupSetAttributeId (h, "ADDLEAF", depth, buf);
	}
	ast_iuptree (h, node->child, depth + 1, 0);
}


void ast_print (struct bitree2 * node, int depth, int leaf)
{
	//Traverse preorder (root, child, next).
	if (!node) {return;}
	char buf [40] = {0};
	struct ast_node * n = container_of (node, struct ast_node, tree);
	snprintf (buf, sizeof (buf), "%02i %02i, %2.*s", depth, leaf, n->p - n->a, n->a);
	for (int i = 0; i < depth; i ++)
	{
		printf("| ");
	}
	printf ("%c\n", *n->a);
	ast_print (node->child, depth + 1, 0);
	ast_print (node->next, depth, leaf + 1);
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
	ASSERT (ih);
	printf("%i %i\n", status, id);
	return IUP_DEFAULT;
}


void showast (struct ast_node * node)
{
	Ihandle * tree = IupTree();
	IupSetAttribute (tree, "SHOWRENAME", "YES");
	IupSetCallback(tree, "SELECTION_CB", (Icallback) function);
	IupSetAttribute (tree, "TITLE","AST");
	IupSetAttribute (tree, "FONT","Courier, 14");
	Ihandle * dlg = IupDialog(IupVbox(tree, NULL));
	IupShow (dlg);
	ast_print (&(node->tree), 0, 0);
	ast_iuptree (tree, &(node->tree), 0, 0);
}

int main (int argc, char * argv [])
{
	ASSERT (argc);
	ASSERT (argv);
	setbuf (stdout, NULL);
	IupOpen (&argc, &argv);

	char const code [] =
	"0 + 1 ^ 2 * 3 ^ 4 + 5";

	struct ast_node * ast = ast_create (code);
	//node1 = ast_add_child (node1, ast_create ("E"));
	//node1 = ast_add_child (node1, ast_create ("E"));

	ast_add (ast, code);


	showast (ast);

	IupMainLoop ();
	IupClose ();

	return EXIT_SUCCESS;
}
