#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <ctype.h>

#include <csc_debug.h>
#include <csc_tcol.h>

enum ast_kind
{
	AST_ERROR = -1,
	AST_UNDEFINED = -2,
	AST_INVALID_TOKEN = -3,
	AST_EXPECTED_NUMBER = -4,
	AST_EXPECTED_TOKEN = -5,
	AST_UNEXPECTED_FACTOR = -6,
	AST_OK = 0,
	AST_PLUS,
	AST_MINUS,
	AST_MUL,
	AST_DIV,
	AST_UNARYMINUS,
	AST_NUMBER,
	AST_EOF,
	AST_OPENP,
	AST_CLOSEP
};

char const * ast_kind_tostr (enum ast_kind kind)
{
	switch (kind)
	{
	case AST_ERROR: return "AST_ERROR";
	case AST_UNDEFINED: return "AST_UNDEFINED";
	case AST_INVALID_TOKEN: return "AST_INVALID_TOKEN";
	case AST_EXPECTED_NUMBER: return "AST_EXPECTED_NUMBER";
	case AST_EXPECTED_TOKEN: return "AST_EXPECTED_TOKEN";
	case AST_UNEXPECTED_FACTOR: return "AST_UNEXPECTED_FACTOR";
	case AST_OK: return "AST_OK";
	case AST_PLUS: return "AST_PLUS";
	case AST_MINUS: return "AST_MINUS";
	case AST_MUL: return "AST_MUL";
	case AST_DIV: return "AST_DIV";
	case AST_UNARYMINUS: return "AST_UNARYMINUS";
	case AST_NUMBER: return "AST_NUMBER";
	case AST_EOF: return "AST_EOF";
	case AST_OPENP: return "AST_OPENP";
	case AST_CLOSEP: return "AST_CLOSEP";
	}
}

struct ast_node;
struct ast_node
{
	enum ast_kind kind;
	struct ast_node * left;
	struct ast_node * right;
};



struct ast_token
{
	enum ast_kind kind;
	double value;
	char * p;
};


void ast_skip (char ** p, int (*f)(int))
{
	while (f (**p)) {(*p) ++;}
}


void ast_get_number (char const * text, struct ast_token * token)
{
	token->kind = AST_NUMBER;
	ast_skip (&token->p, isspace);
	char * p = token->p;
	ast_skip (&token->p, isdigit);
	if (token->p [0] == '.') {token->p ++;}
	ast_skip (&token->p, isdigit);
	if(p == token->p)
	{
		token->kind = AST_EXPECTED_NUMBER;
		return;
	}
	token->value = atof (p);
}

void ast_next (char const * text, struct ast_token * token)
{
	ast_skip (&token->p, isspace);
	token->value = 0;
	if (token->p [0] == 0)
	{
		token->kind = AST_EOF;
		return;
	}
	if (isdigit (token->p [0]))
	{
		ast_get_number (text, token);
		return;
	}
	switch (token->p [0])
	{
	case '+': token->kind = AST_PLUS; break;
	case '-': token->kind = AST_MINUS; break;
	case '*': token->kind = AST_MUL; break;
	case '/': token->kind = AST_DIV; break;
	case '(': token->kind = AST_OPENP; break;
	case ')': token->kind = AST_CLOSEP; break;
	default:
	token->kind = AST_INVALID_TOKEN;
	return;
	}
	token->p ++;
}

void ast_expression (char const * text, struct ast_token * token);

void ast_match (char const * text, struct ast_token * token, char expected)
{
	if (token->kind < 0) {return;}
	if (token->p [-1] == expected)
	{
		ast_next (text, token);
	}
	else
	{
		token->kind = AST_EXPECTED_TOKEN;
	}
}

void ast_factor (char const * text, struct ast_token * token)
{
	if (token->kind < 0) {return;}
	switch(token->kind)
	{
	case AST_OPENP:
	ast_next (text, token);
	ast_expression (text, token);
	ast_match (text, token, ')');
	break;

	case AST_MINUS:
	ast_next (text, token);
	ast_factor (text, token);
	break;

	case AST_NUMBER:
	ast_next (text, token);
	break;

	default:
	token->kind = AST_UNEXPECTED_FACTOR;
	}
}

void ast_term1 (char const * text, struct ast_token * token)
{
	if (token->kind < 0) {return;}
	switch (token->kind)
	{
	case AST_MUL:
	ast_next (text, token);
	ast_factor (text, token);
	ast_term1 (text, token);
	break;
	case AST_DIV:
	ast_next (text, token);
	ast_factor (text, token);
	ast_term1 (text, token);
	break;
	default:
	break;
	}
}

void ast_term (char const * text, struct ast_token * token)
{
	if (token->kind < 0) {return;}
	ast_factor (text, token);
	ast_term1 (text, token);
}

void ast_expression1 (char const * text, struct ast_token * token)
{
	if (token->kind < 0) {return;}
	switch(token->kind)
	{
	case AST_PLUS:
	ast_next (text, token);
	ast_term (text, token);
	ast_expression1(text, token);
	break;

	case AST_MINUS:
	ast_next (text, token);
	ast_term (text, token);
	ast_expression1(text, token);
	break;

	default:
	break;
	}
}

void ast_expression (char const * text, struct ast_token * token)
{
	if (token->kind < 0) {return;}
	ast_term (text, token);
	ast_expression1 (text, token);
}


void parse (char const * text)
{
	struct ast_token token = {0};
	token.p = text;
	ast_next (text, &token);
	ast_expression (text, &token);
	if (token.kind < 0)
	{
		fprintf (stderr, "%s : %s %c(%c)%c\n", text, ast_kind_tostr (token.kind), token.p [-1], token.p [0], token.p [1]);
	}
}


int main (int argc, char * argv [])
{
	parse ("1+2+3+4");
	parse ("1*2*3*4");
	parse ("1-2-3-4");
	parse ("1/2/3/4");
	parse ("1*2+3*4");
	parse ("1+2*3+4");
	parse ("(1+2)*(3+4)");
	parse ("1+(2*3)*(4+5)");
	parse ("1+(2*3)/4+5");
	parse ("5/(4+3)/2");
	parse ("1 + 2.5");
	parse ("125");
	parse ("-1");
	parse ("-1+(-2)");
	parse ("-1+(-2.0)");
	parse ("   1*2,5");
	parse ("   1*2.5e2");
	parse ("M1 + 2.5");
	parse ("1 + 2&5");
	parse ("1 * 2.5.6");
	parse ("1 ** 2.5");
	parse ("*1 / 2.5");
	return 0;
}
