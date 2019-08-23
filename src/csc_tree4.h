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


/*
https://en.wikipedia.org/wiki/Left-child_right-sibling_binary_tree
TODO:
	This is currently used for AST. Can this be more generic?
*/
struct csc_tree4;
struct csc_tree4
{
	struct csc_tree4 * prev;
	struct csc_tree4 * next;
	struct csc_tree4 * parent;
	struct csc_tree4 * child;
};

void csc_tree4_addchild (struct csc_tree4 * parent, struct csc_tree4 * child)
{
	parent->child = child;
	child->parent = parent;
}

void csc_tree4_addsibling (struct csc_tree4 * sibling0, struct csc_tree4 * sibling1)
{
	sibling0->next = sibling1;
	sibling1->prev = sibling0;
	sibling1->parent = sibling0->parent;
}

void csc_tree4_addparent (struct csc_tree4 * node, struct csc_tree4 * newnode)
{
	newnode->child = node;
	newnode->parent = node->parent;
	newnode->next = node->next;
	newnode->prev = node->prev;
	if (node->next) {node->next->prev = newnode;}
	if (node->prev) {node->prev->next = newnode;}
	if (node->parent && (node->parent->child == node)) {node->parent->child = newnode;}
	node->parent = newnode;
	node->next = 0;
	node->prev = 0;
}
