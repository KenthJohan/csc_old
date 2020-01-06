/*
MIT License

Copyright (c) 2019 CSC Project

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once


enum csc_tok_c_type
{
	CSC_TOK_C_VOID = 256,
	CSC_TOK_C_CONST,
	CSC_TOK_C_INT,
	CSC_TOK_C_IDENTIFIER,
	CSC_TOK_C_LITERAL_INTEGER,
	CSC_TOK_C_IF
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
	case CSC_TOK_C_VOID: return "VOID";
	case CSC_TOK_C_CONST: return "CONST";
	case CSC_TOK_C_IDENTIFIER: return "IDENTIFIER";
	case CSC_TOK_C_INT: return "INT";
	case CSC_TOK_C_LITERAL_INTEGER: return "LITERAL_INTEGER";
	}
	return 0;
}
