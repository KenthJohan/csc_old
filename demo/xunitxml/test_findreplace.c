#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>

char * stpcpy (char * dst, char const * src)
{
	assert (dst);
	assert (src);
	size_t len = strlen (src);
	return (char*) memcpy (dst, src, len + 1) + len;
}

//Assume the (src) string are     (<a> <needle> <b>)
//Assume the (dst) string becomes (<a> <text>   <b>)
char * str_findreplace_unsafe (char const * src, char * dst, char const * needle, char const * text)
{
	assert (src);
	assert (dst);
	assert (needle);
	assert (text);
	assert (src != dst);
	//src: (src)<a><needle><b>
	//dst: (dst)<NULL>
	char const * a = strstr (src, needle);
	if (a == NULL) {return NULL;}
	ptrdiff_t n = a - src;
	assert (n >= 0);
	memcpy (dst, src, (size_t) n);
	//src: (src)<a><needle><b>
	//dst: (dst)<a>
	src += n;
	//src: <a>(src)<needle><b>
	//des: (dst)<a>
	src += strlen (needle);
	//src: <a><needle>(src)<b>
	//des: (dst)<a>
	dst += n;
	//src: <a><needle>(src)<b>
	//des: <a>(dst)
	dst = stpcpy (dst, text);
	//src: <a><needle>(src)<b>
	//des: <a><text>(dst)
	dst = stpcpy (dst, src);
	//src: <a><text>(src)<b>
	//des: <a><text><b>(dst)
	return dst;
}

char * str_findreplace (char * src, char * dst, size_t count, char const * needle, char const * text)
{
	assert (src);
	assert (dst);
	assert (needle);
	assert (text);
	assert (strnlen (src, count) < count);
	assert (strnlen (dst, count) < count);
	assert (strlen (src) + strlen (text) < count);
	return str_findreplace_unsafe (src, dst, needle, text);
}


char * malloc_file1 (char const * filename, long * length)
{
	FILE * file = NULL;
	char * buffer = NULL;
	if (filename == NULL) {goto error;}
	file = fopen (filename, "rb");
	if (file == NULL) {goto error;}
	int r;
	r = fseek (file, 0, SEEK_END);
	if (r != 0) {goto error;}
	(*length) = ftell (file);
	if ((*length) < 0) {goto error;}
	r = fseek (file, 0, SEEK_SET);
	if (r != 0) {goto error;}
	buffer = (char *) malloc ((unsigned) (*length) + 1);
	if (buffer == NULL) {goto error;}
	memset (buffer, 0, (unsigned) (*length) + 1);
	//buffer [length + 1] = 0;
	if (*length > 0)
	{
		size_t n = fread (buffer, (unsigned) (*length), 1, file);
		if (n != 0) {goto error;}
	}
	fclose (file);
	return buffer;
error:
	if (file) {fclose (file);}
	if (buffer) {free (buffer);}
	return NULL;
}


int read_whole_file (char const * filename, char * buffer, size_t count)
{
	assert (filename);
	assert (buffer);
	int r = -1;
	FILE * file = NULL;
	if (filename == NULL) {goto error;}
	file = fopen (filename, "rb");
	if (file == NULL) {goto error;}
	r = fseek (file, 0, SEEK_END);
	if (r != 0) {goto error;}
	long n = ftell (file);
	if (n < 0) {r=-1;goto error;}
	if (n > (long)count) {r=-1;goto error;}
	r = fseek (file, 0, SEEK_SET);
	if (r != 0) {goto error;}
	r = (int)fread (buffer, (unsigned) n, 1, file);
	if (r != 0) {goto error;}
error:
	if (file) {fclose (file);}
	return r;
}




#define BUF_COUNT 100

int main (int argc, char const * argv [])
{
	setbuf (stdout, NULL);

	char buf0 [BUF_COUNT] = {0};
	char buf1 [BUF_COUNT] = {0};
	read_whole_file ("template.txt", buf0, BUF_COUNT);
	printf ("buf0: %s\n", buf0);
	printf ("buf1: %s\n", buf1);

	str_findreplace (buf0, buf1, BUF_COUNT, "$(name)", "world");
	strcpy (buf0, buf1);
	printf ("buf0: %s\n", buf0);
	printf ("buf1: %s\n", buf1);

	str_findreplace (buf0, buf1, BUF_COUNT, "$(a)", "washing machone");
	strcpy (buf0, buf1);
	printf ("buf0: %s\n", buf0);
	printf ("buf1: %s\n", buf1);

	str_findreplace (buf0, buf1, BUF_COUNT, "$(b)", "telphone");
	strcpy (buf0, buf1);
	printf ("buf0: %s\n", buf0);
	printf ("buf1: %s\n", buf1);

	return EXIT_SUCCESS;
}

