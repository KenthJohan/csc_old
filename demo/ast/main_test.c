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
#include <csc_str.h>
#include <csc_tok_c.h>
#include <csc_tree4.h>


enum ast_nodetype
{
	AST_START,
	AST_ROOT_CHILD,
	AST_ROOT_CHILD0,
	AST_UNKNOWN,
	AST_FUNCTION,
	AST_FUNCTION_HEADER,
	AST_FUNCTION_IDENTIFIER,
	AST_FUNCTION_RETURNTYPE,
	AST_FUNCTION0,
	AST_PARAM,
	AST_PARAM_TYPE,
	AST_PARAM_IDENTIFIER,
	AST_DECLARATION0,
	AST_DECLARATION_VARIABLE,
	AST_NAME,
	AST_TYPE,
	AST_INITIALIZATION,
	AST_PARAMS, //PARAMS, PARAM | PARAM | EMPTY
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


char const * ast_nodetype_tostr (enum ast_nodetype t)
{
	switch (t)
	{
	case AST_START: return "START";
	case AST_UNKNOWN: return "UNKNOWN";
	case AST_IDENTIFIER: return "IDENTIFIER";
	case AST_ROOT_CHILD: return "ROOT_CHILD";
	case AST_ROOT_CHILD0: return "ROOT_CHILD0";
	case AST_PARAM: return "PARAM";
	case AST_PARAM_TYPE: return "PARAM_TYPE";
	case AST_PARAM_IDENTIFIER: return "PARAM_IDENTIFIER";
	case AST_DECLARATION0: return "DECLARATION0";
	case AST_FUNCTION: return "FUNCTION";
	case AST_FUNCTION_HEADER: return "FUNCTION_HEADER";
	case AST_FUNCTION_IDENTIFIER: return "FUNCTION_IDENTIFIER";
	case AST_FUNCTION_RETURNTYPE: return "FUNCTION_RETURNTYPE";
	case AST_FUNCTION0: return "FUNCTION0";
	case AST_NAME: return "NAME";
	case AST_TYPE: return "TYPE";
	case AST_PARAMS: return "PARAMS";
	case AST_PARAMS0: return "PARAMS0";
	default:return "";
	}
}


struct ast_node;
struct ast_node
{
	enum ast_nodetype kind;
	struct csc_tree4 tree;
	char const * name;
};


void ast_add (struct ast_node * root, struct ast_node ** nextroot, char const * code)
{
	struct ast_node * node = NULL;
again:
	switch (root->kind)
	{
	case AST_FUNCTION_HEADER:
	case AST_START:
		node = calloc (1, sizeof (struct ast_node));
		node->kind = AST_UNKNOWN;
		node->name = "";
		csc_tree4_addsibling (&root->tree, &node->tree);
		root = node;
		node = calloc (1, sizeof (struct ast_node));
		node->kind = AST_UNKNOWN;
		node->name = code;
		csc_tree4_addchild (&root->tree, &node->tree);
		*nextroot = node;
		break;

	case AST_UNKNOWN:
		if (root->tree.parent && (root->tree.parent->child_count == 2) && (strcmp (code, "(") == 0))
		{
			node = container_of (root->tree.parent, struct ast_node, tree);
			node->kind = AST_FUNCTION;
			node = container_of (root->tree.parent->child, struct ast_node, tree);
			node->kind = AST_FUNCTION_RETURNTYPE;
			node = container_of (root->tree.parent->child->next, struct ast_node, tree);
			node->kind = AST_FUNCTION_IDENTIFIER;
			node = calloc (1, sizeof (struct ast_node));
			node->kind = AST_PARAMS;
			node->name = code;
			csc_tree4_addsibling (&root->tree, &node->tree);
			*nextroot = node;
		}
		else
		{
			node = calloc (1, sizeof (struct ast_node));
			node->kind = AST_UNKNOWN;
			node->name = code;
			csc_tree4_addsibling (&root->tree, &node->tree);
			*nextroot = node;
		}
		break;

	case AST_FUNCTION:
		if (strcmp (code, ";") == 0)
		{
			root->kind = AST_FUNCTION_HEADER;
		}
		break;

	//PARAMS | PARAM | EMPTY
	case AST_PARAMS:
		if (strcmp (code, ")") == 0)
		{
			ASSERT (root->tree.parent);
			*nextroot = container_of (root->tree.parent, struct ast_node, tree);
		}
		else
		{
			node = calloc (1, sizeof (struct ast_node));
			node->kind = AST_PARAM;
			node->name = "";
			csc_tree4_addchild (&root->tree, &node->tree);
			root = node;
			goto again;
		}
		break;

	case AST_PARAM:
		node = calloc (1, sizeof (struct ast_node));
		node->kind = AST_PARAM_TYPE;
		node->name = code;
		csc_tree4_addchild (&root->tree, &node->tree);
		*nextroot = node;
		break;

	case AST_PARAM_TYPE:
		node = calloc (1, sizeof (struct ast_node));
		node->kind = AST_PARAM_IDENTIFIER;
		node->name = code;
		csc_tree4_addsibling (&root->tree, &node->tree);
		*nextroot = node;
		break;

	case AST_PARAM_IDENTIFIER:
		if (root->tree.parent && root->tree.parent->parent && root->tree.parent->parent->parent && strcmp (code, ")") == 0)
		{
			*nextroot = container_of (root->tree.parent->parent->parent, struct ast_node, tree);
		}
		if (root->tree.parent && strcmp (code, ",") == 0)
		{
			root = container_of (root->tree.parent, struct ast_node, tree);
			node = calloc (1, sizeof (struct ast_node));
			node->kind = AST_PARAM;
			node->name = "";
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
	snprintf (buf, sizeof (buf), TCOL(TCOL_BOLD,TCOL_DEFAULT,TCOL_DEFAULT) "%s" TCOL_RST " [%s] (%ic)", ast_nodetype_tostr (n->kind), n->name, n->tree.child_count);
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

	// int hello1 (hello2 hello3 (
	ast_print (&root->tree, 0, 0, 0);puts("");
	ast_add (p, &p, "int");
	ast_print (&root->tree, 0, 0, 0);puts("");
	ast_add (p, &p, "hello1");
	ast_print (&root->tree, 0, 0, 0);puts("");
	ast_add (p, &p, "(");
	ast_print (&root->tree, 0, 0, 0);puts("");
	ast_add (p, &p, "float");
	ast_add (p, &p, "hello3");
	ast_add (p, &p, ",");
	ast_print (&root->tree, 0, 0, 0);puts("");
	ast_add (p, &p, "int");
	ast_print (&root->tree, 0, 0, 0);puts("");
	ast_add (p, &p, "count");
	ast_print (&root->tree, 0, 0, 0);puts("");
	ast_add (p, &p, ")");
	ast_print (&root->tree, 0, 0, 0);puts("");
	ast_add (p, &p, ";");
	ast_print (&root->tree, 0, 0, 0);puts("");
	ast_add (p, &p, "int");
	ast_print (&root->tree, 0, 0, 0);puts("");
	ast_add (p, &p, "hello1");
	ast_print (&root->tree, 0, 0, 0);puts("");
	ast_add (p, &p, "(");
	ast_print (&root->tree, 0, 0, 0);puts("");
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
