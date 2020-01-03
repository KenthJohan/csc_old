#include <stdio.h>
#include <stdarg.h>
#define _GNU_SOURCE
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
#include <csc_str.h>
#include <csc_tok_c.h>
#include <csc_tree4.h>





enum ast_nodetype
{
	AST_START,
	AST_ROOT_CHILD,
	AST_ROOT_CHILD0,
	AST_UNKNOWN,
	AST_FUNCTION_UNKNOWN,
	AST_FUNCTION_HEADER,
	AST_FUNCTION_IMPL,
	AST_FUNCTION_IMPL_UNKNOWN,
	AST_FUNCTION_BODY,
	AST_FUNCTION_BODY_UNKNOWN,
	AST_FUNCTION_IDENTIFIER,
	AST_FUNCTION_RETURNTYPE,
	AST_FUNCTION0,
	AST_PARAM_TYPE,
	AST_PARAM_IDENTIFIER,
	AST_DECLARATION0,
	AST_DECLARATION_VARIABLE,
	AST_NAME,
	AST_TYPE,
	AST_INITIALIZATION,
	AST_PARAM,
	AST_PARAM_UNKNOWN,
	AST_PARAMS, //PARAMS, PARAM | PARAM | EMPTY
	AST_PARAMS_UNKOWN, //PARAMS, PARAM | PARAM | EMPTY
	AST_PARAMS0,
	AST_FUNCTION_ARGUMENT,
	AST_FUNCTION_RETURN_TYPE,
	AST_FUNCTION_NAME,
	AST_CALL,
	AST_CALL_ARGUMENTS,
	AST_IDENTIFIER,
	AST_IDENTIFIER_FUNCTION,
	AST_LITERAL,
	AST_LITERAL_INTEGER,
	AST_LITERAL_STRING,
	AST_EXPRESSION,
	AST_VOID,
	AST_ADD,
	AST_SUB,
	AST_MUL,
	AST_UNSIGNED,
	AST_SIGNED
};


struct token
{
	int lin;
	int col;
	int tok;
	char const * a;
	char const * b;
};


void tok_next (struct token * tok)
{
again:
	tok->a = tok->b;
	switch (*tok->b)
	{
	case '\0':
		tok->tok = 0;
		return;
	case ' ':
		tok->col ++;
		tok->b ++;
		goto again;
	case '\r':
		tok->b ++;
		goto again;
	case '\n':
		tok->col = 0;
		tok->lin ++;
		tok->b ++;
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
	case ';':
		tok->tok = *tok->b;
		tok->b ++;
		tok->col ++;
		return;
	}
	if (0) {}
	else if (csc_next_literal (&tok->b, &tok->col))
	{
		tok->tok = CSC_TOK_LITERAL_INTEGER;
	}
	else if (csc_str_next_cmp (&tok->b, &tok->col, "void"))
	{
		tok->tok = CSC_TOK_C_CONST;
	}
	else if (csc_str_next_cmp (&tok->b, &tok->col, "const"))
	{
		tok->tok = CSC_TOK_C_VOID;
	}
	else if (csc_next_indentifer (&tok->b, &tok->col))
	{
		tok->tok = CSC_TOK_IDENTIFIER;
	}
	else if (csc_next_indentifer (&tok->b, &tok->col))
	{
		tok->tok = CSC_TOK_INT;
	}
	return;
}





char const * ast_nodetype_tostr (enum ast_nodetype t)
{
	switch (t)
	{
	case AST_START: return "START";
	case AST_UNKNOWN: return "?";
	case AST_IDENTIFIER: return "ID";
	case AST_ROOT_CHILD: return "ROOT_CHILD";
	case AST_ROOT_CHILD0: return "ROOT_CHILD0";
	case AST_PARAM: return "FUN_PAR";
	case AST_PARAM_UNKNOWN: return "FUN_PAR?";
	case AST_PARAM_TYPE: return "FUN_PAR_TYPE";
	case AST_PARAM_IDENTIFIER: return "FUN_PAR_ID";
	case AST_PARAMS_UNKOWN: return "FUN_PARS?";
	case AST_PARAMS: return "FUN_PARS";
	case AST_DECLARATION0: return "DECLARATION0";
	case AST_FUNCTION_UNKNOWN: return "FUN?";
	case AST_FUNCTION_HEADER: return "FUN_HEAD";
	case AST_FUNCTION_IMPL: return "FUN_IMPL";
	case AST_FUNCTION_IMPL_UNKNOWN: return "FUN_IMPL?";
	case AST_FUNCTION_BODY: return "FUN_BODY";
	case AST_FUNCTION_BODY_UNKNOWN: return "FUN_BODY?";
	case AST_FUNCTION_IDENTIFIER: return "FUN_ID";
	case AST_FUNCTION_RETURNTYPE: return "FUN_RET";
	case AST_FUNCTION0: return "FUNCTION0";
	case AST_NAME: return "NAME";
	case AST_TYPE: return "TYPE";
	default:return "";
	}
}


struct ast_node;
struct ast_node
{
	enum ast_nodetype kind;
	struct csc_tree4 tree;
	struct token tok;
};


void ast_add (struct ast_node * root, struct ast_node ** nextroot, struct token tok)
{
	struct ast_node * node = NULL;
again:
	switch (root->kind)
	{
	case AST_FUNCTION_HEADER:
	case AST_START:
		node = calloc (1, sizeof (struct ast_node));
		node->kind = AST_UNKNOWN;
		csc_tree4_addsibling (&root->tree, &node->tree);
		root = node;
		node = calloc (1, sizeof (struct ast_node));
		node->kind = AST_UNKNOWN;
		node->tok = tok;
		csc_tree4_addchild (&root->tree, &node->tree);
		*nextroot = node;
		break;

	case AST_UNKNOWN:
		if (root->tree.parent && (root->tree.parent->child_count == 2) && (tok.tok == '('))
		{
			node = container_of (root->tree.parent, struct ast_node, tree);
			node->kind = AST_FUNCTION_UNKNOWN;
			node = container_of (root->tree.parent->child, struct ast_node, tree);
			node->kind = AST_FUNCTION_RETURNTYPE;
			node = container_of (root->tree.parent->child->next, struct ast_node, tree);
			node->kind = AST_FUNCTION_IDENTIFIER;
			node = calloc (1, sizeof (struct ast_node));
			node->kind = AST_PARAMS_UNKOWN;
			node->tok = tok;
			csc_tree4_addsibling (&root->tree, &node->tree);
			*nextroot = node;
		}
		else
		{
			node = calloc (1, sizeof (struct ast_node));
			node->kind = AST_UNKNOWN;
			node->tok = tok;
			csc_tree4_addsibling (&root->tree, &node->tree);
			*nextroot = node;
		}
		break;

	case AST_FUNCTION_UNKNOWN:
		break;

	case AST_FUNCTION_IMPL_UNKNOWN:
		if (tok.tok == '}')
		{
			root->kind = AST_FUNCTION_IMPL;
			ASSERT (root->tree.parent);
			node = container_of (root->tree.parent, struct ast_node, tree);
			*nextroot = node;
		}
		break;

	//PARAMS | PARAM | EMPTY
	case AST_PARAMS_UNKOWN:
		if (tok.tok == ')')
		{
			root->kind = AST_PARAMS;
		}
		else
		{
			node = calloc (1, sizeof (struct ast_node));
			node->kind = AST_PARAM_UNKNOWN;
			csc_tree4_addchild (&root->tree, &node->tree);
			root = node;
			goto again;
		}
		break;

	case AST_PARAMS:
		if (tok.tok == '{')
		{
			node = container_of (root->tree.parent, struct ast_node, tree);
			node->kind = AST_FUNCTION_IMPL_UNKNOWN;
			node = calloc (1, sizeof (struct ast_node));
			node->kind = AST_FUNCTION_BODY_UNKNOWN;
			node->tok = tok;
			csc_tree4_addchild (&root->tree, &node->tree);
			*nextroot = node;
		}
		else if (tok.tok == ';')
		{
			node = container_of (root->tree.parent, struct ast_node, tree);
			node->kind = AST_FUNCTION_HEADER;
			*nextroot = node;
		}
		break;

	case AST_PARAM_UNKNOWN:
		node = calloc (1, sizeof (struct ast_node));
		node->kind = AST_PARAM_TYPE;
		node->tok = tok;
		csc_tree4_addchild (&root->tree, &node->tree);
		*nextroot = node;
		break;

	case AST_PARAM_TYPE:
		node = calloc (1, sizeof (struct ast_node));
		node->kind = AST_PARAM_IDENTIFIER;
		node->tok = tok;
		csc_tree4_addsibling (&root->tree, &node->tree);
		*nextroot = node;
		break;

	case AST_PARAM_IDENTIFIER:
		if (root->tree.parent && root->tree.parent->parent && tok.tok == ')')
		{
			node = container_of (root->tree.parent, struct ast_node, tree);
			node->kind = AST_PARAM;
			node = container_of (root->tree.parent->parent, struct ast_node, tree);
			node->kind = AST_PARAMS;
			*nextroot = node;
		}
		if (root->tree.parent && tok.tok == ',')
		{
			root = container_of (root->tree.parent, struct ast_node, tree);
			root->kind = AST_PARAM;
			node = calloc (1, sizeof (struct ast_node));
			node->kind = AST_PARAM_UNKNOWN;
			csc_tree4_addsibling (&root->tree, &node->tree);
			*nextroot = node;
		}
		break;

	default:
		break;
	}
}


void ast_print (struct csc_tree4 * node, int depth, int leaf, uint32_t indent)
{
	char vertical[] = TCOL(TCOL_NORMAL,TCOL_GREEN,TCOL_DEFAULT) "\u2502" TCOL_RST;
	char cross[] = TCOL(TCOL_NORMAL,TCOL_GREEN,TCOL_DEFAULT) "\u251C" TCOL_RST;
	char corner[] = TCOL(TCOL_NORMAL,TCOL_GREEN,TCOL_DEFAULT) "\u2514" TCOL_RST;
	char dash[] = TCOL(TCOL_NORMAL,TCOL_GREEN,TCOL_DEFAULT) "\u2500" TCOL_RST;

	//Traverse preorder (root, child, next).
	if (!node) {return;}
	char buf [100] = {0};
	struct ast_node * n = container_of (node, struct ast_node, tree);
	//snprintf (buf, sizeof (buf), "%02i %02i, %2.*s", depth, leaf, (int)(n->p - n->a), n->a);
	snprintf (buf, sizeof (buf), TCOL(TCOL_BOLD,TCOL_DEFAULT,TCOL_DEFAULT) "%s" TCOL_RST " [%.*s] (%ic)", ast_nodetype_tostr (n->kind), n->tok.b-n->tok.a, n->tok.a, n->tree.child_count);

	for (int i = 0; i < depth; i ++)
	{
		if (indent & (1 << i))
		{
			printf ("%s", vertical);
		}
		else
		{
			putc (' ', stdout);
		}
		putc (' ', stdout);
		putc (' ', stdout);
	}
	if (node->next)
	{
		printf ("%s", cross);
		indent |= (1 << depth);
	}
	if (node->next == NULL)
	{
		printf ("%s", corner);
		indent &= ~(1 << depth);
	}
	//printf ("%c\n", *n->a);

	//putc (dash, stdout);
	printf ("%s", dash);
	puts (buf);
	ast_print (node->child, depth + 1, 0, indent);
	ast_print (node->next, depth, leaf + 1, indent);
}


void ast_print_nonrecursive (struct csc_tree4 const * node)
{
	char vertical[] = TCOL(TCOL_NORMAL,TCOL_GREEN,TCOL_DEFAULT) "\u2502" TCOL_RST;
	char cross[] = TCOL(TCOL_NORMAL,TCOL_GREEN,TCOL_DEFAULT) "\u251C" TCOL_RST;
	char corner[] = TCOL(TCOL_NORMAL,TCOL_GREEN,TCOL_DEFAULT) "\u2514" TCOL_RST;
	char dash[] = TCOL(TCOL_NORMAL,TCOL_GREEN,TCOL_DEFAULT) "\u2500" TCOL_RST;
	char buf [100] = {0};
	struct ast_node const * n;
	int depth = 0;
	//int leaf = 0;
	uint32_t indent = 0;
again:
	n = container_of (node, struct ast_node const, tree);
	//snprintf (buf, sizeof (buf), "%02i %02i, %2.*s", depth, leaf, (int)(n->p - n->a), n->a);
	snprintf (buf, sizeof (buf), TCOL(TCOL_BOLD,TCOL_DEFAULT,TCOL_DEFAULT) "%s" TCOL_RST " [%.*s] (%ic)", ast_nodetype_tostr (n->kind), n->tok.b-n->tok.a, n->tok.a, n->tree.child_count);
	for (int i = 0; i < depth; i ++)
	{
		if (indent & (1 << i))
		{
			printf ("%s", vertical);
		}
		else
		{
			putc (' ', stdout);
		}
		putc (' ', stdout);
		putc (' ', stdout);
	}
	if (node->next)
	{
		printf ("%s", cross);
		indent |= (1 << depth);
	}
	if (node->next == NULL)
	{
		printf ("%s", corner);
		indent &= ~(1 << depth);
	}
	printf ("%s", dash);
	puts (buf);

	if (node->child)
	{
		depth += 1;
		node = node->child;
		goto again;
	}
	else if (node->next)
	{
		node = node->next;
		goto again;
	}
	else if (node->parent && (node->child == NULL) && (node->next == NULL))
	{
		do
		{
			depth -= 1;
			node = node->parent;
		}
		while (node && (node->next == NULL));
		if (node && node->next)
		{
			node = node->next;
			goto again;
		}
	}
}


char *my_strndup(char *str, int chars)
{
	char *buffer;
	int n;

	buffer = (char *) malloc(chars +1);
	if (buffer)
	{
		for (n = 0; ((n < chars) && (str[n] != 0)) ; n++) buffer[n] = str[n];
		buffer[n] = 0;
	}

	return buffer;
}


int main (int argc, char * argv [])
{
	ASSERT (argc);
	ASSERT (argv);
	setbuf (stdout, NULL);
	setlocale (LC_CTYPE, "");

	struct ast_node * root = calloc (1, sizeof (struct ast_node));
	root->kind = AST_START;
	struct ast_node * p = root;
	struct ast_node * t;

	char * code =
		"int hello (float arg1, int count);"
		"int hello1 (){}";

	struct token tok;
	tok.b = code;
	do
	{
		tok_next (&tok);
		fwrite (tok.a, 1, (tok.b-tok.a), stdout);puts("");
		ast_add (p, &p, tok);
	}
	while (tok.tok);

	ast_print_nonrecursive (&root->tree);
	puts("");

	return 0;

	/*
	// int hello1 (hello2 hello3 (
	//ast_print (&root->tree, 0, 0, 0);puts("");
	ast_add (p, &p, "int");//ast_print (&root->tree, 0, 0, 0);puts("");
	ast_add (p, &p, "hello1");//ast_print (&root->tree, 0, 0, 0);puts("");
	ast_add (p, &p, "(");//ast_print (&root->tree, 0, 0, 0);puts("");
	ast_add (p, &p, "float");
	ast_add (p, &p, "hello3");
	ast_add (p, &p, ",");//ast_print (&root->tree, 0, 0, 0);puts("");
	ast_add (p, &p, "int");//ast_print (&root->tree, 0, 0, 0);puts("");
	ast_add (p, &p, "count");//ast_print (&root->tree, 0, 0, 0);puts("");
	ast_add (p, &p, ")");//ast_print (&root->tree, 0, 0, 0);puts("");
	ast_add (p, &p, ";");//ast_print (&root->tree, 0, 0, 0);puts("");
	ast_add (p, &p, "int");//ast_print (&root->tree, 0, 0, 0);puts("");
	ast_add (p, &p, "hello1");//ast_print (&root->tree, 0, 0, 0);puts("");
	ast_add (p, &p, "(");//ast_print (&root->tree, 0, 0, 0);puts("");
	ast_add (p, &p, ")");//ast_print (&root->tree, 0, 0, 0);puts("");
	ast_add (p, &p, "{");//ast_print (&root->tree, 0, 0, 0);puts("");
	ast_add (p, &p, "}");ast_print (&root->tree, 0, 0, 0);puts("");
	ast_print_nonrecursive (&root->tree);
	*/

	//ast_print (&root->tree, 0, 0, 0);
	//printf ("%s [%s]\n", ast_nodetype_tostr (p->kind), p->a);
	/*
	ast_add (p, &p, "int");
	ast_add (p, &p, "double");
	ast_add (p, &p, ")");
	ast_add (p, &p, ";");
	ast_add (p, &p, "foo1");
	ast_add (p, &p, "(");
	ast_add (p, &p, "float");
	ast_add (p, &p, "float");
	ast_add (p, &p, ";");
	ast_print (&root->tree, 0, 0, 0);
	*/

	return EXIT_SUCCESS;
}
