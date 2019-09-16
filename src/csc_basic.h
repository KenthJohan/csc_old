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
#include <stddef.h> //offsetof

#define MAX(a,b) ((a)>(b)?(a):(b))
#define MIN(a,b) ((a)<(b)?(a):(b))
//#define INSIDE(x, a, b) ((a) <= (x) && (x) <= (b))
#define CLAMP(x, x0, x1) (((x) > (x1)) ? (x1) : (((x) < (x0)) ? (x0) : (x)))
#define SWAP(t, a, b) do{t c = (b); (b) = (a); (a) = (c);} while (0)
#define SWAPX(a, b)	((a)^=(b),(b)^=(a),(a)^=(b))

#define container_of(ptr, type, member) ((type *)(void *)((char *)(ptr) - offsetof(type, member)))
