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

#include <errno.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <io.h>


int csc_blksread (int fd, char * block [], unsigned block_count, unsigned block_size)
{
	//The (bz) is amount of memory left in the block:
	unsigned bz = 0;

	//The (bn) is amount of blocks left:
	unsigned bn = block_count;

	//The current block memory:
	char * b = NULL;

	while (1)
	{
		//Check if no more blocks is available
		if (bn == 0)
		{
			//Out of memory
			errno = ENOMEM;
			return -1;
		}

		//Check if the whole block has been filled:
		//Check if this is the first iteration of the loop:
		if (bz == 0)
		{
			//Start using a new block:
			//Check if the user has not allocated memory for the new block:
			if ((*block) == NULL)
			{
				(*block) = (char *)calloc (block_size, sizeof (char));
				//(*block) = (char *)calloc (sizeof (char), block_size);
				//(*block) = (char *)malloc (block_size);
				assert ((*block));
				b = (*block);
				block ++;
				bn --;
			}
			bz = block_size;
		}

		assert (bz > 0);
		assert (bz <= block_size);
		assert (b);
		int n = read (fd, b, (unsigned) bz);
		printf ("n %i\n", n);

		//Check if end of file has been reached:
		if (n == 0)
		{
			return (int)block_count - (int)bn;
		}

		//Check if read failed:
		else if (n < 0)
		{
			return -1;
		}
		b += n;
		bz -= (unsigned)n;
	}
}
