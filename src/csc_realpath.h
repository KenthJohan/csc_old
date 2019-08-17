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

#ifndef _WIN32
#define csc_realpath realpath
#else
#include <windows.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#include <sys/stat.h>

char * csc_realpath (char const * path, char resolved_path [PATH_MAX])
{
	if (path == NULL || resolved_path == NULL)
	{
		errno = EINVAL;
		return NULL;
	}

	//https://docs.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-getfullpathnamea
	size_t size = GetFullPathNameA (path, PATH_MAX, resolved_path, 0);

	if (size > PATH_MAX)
	{
		errno = ENAMETOOLONG;
		return NULL;
	}

	//If the function fails for any other reason, the return value is zero.
	//To get extended error information, call GetLastError.
	if (size == 0)
	{
		//https://docs.microsoft.com/en-us/windows/win32/api/errhandlingapi/nf-errhandlingapi-getlasterror
		//https://docs.microsoft.com/en-us/windows/win32/debug/system-error-codes--0-499-
		switch (GetLastError())
		{
		case ERROR_FILE_NOT_FOUND:
			errno = ENOENT;
			return NULL;

		case ERROR_PATH_NOT_FOUND:
		case ERROR_INVALID_DRIVE:
			errno = ENOTDIR;
			return NULL;

		case ERROR_ACCESS_DENIED:
			errno = EACCES;
			return NULL;

		default:
			errno = EIO;
			return NULL;
		}
	}

	return resolved_path;
}
#endif
