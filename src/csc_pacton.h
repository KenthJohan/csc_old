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

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>

#include "csc_debug.h"

#define PACTON_STR(A) #A
#define PACTON_XSTR(A) PACTON_STR(A)


enum pacton_byteorder
{
	PACTON_BYTEORDER_LE,
	PACTON_BYTEORDER_BE,
	PACTON_BYTEORDER__N,
	PACTON_BYTEORDER_UNKNOWN
};

enum pacton_primtype
{
	PACTON_PRIMTYPE_I8,
	PACTON_PRIMTYPE_I16,
	PACTON_PRIMTYPE_I32,
	PACTON_PRIMTYPE_I64,
	PACTON_PRIMTYPE_U8,
	PACTON_PRIMTYPE_U16,
	PACTON_PRIMTYPE_U32,
	PACTON_PRIMTYPE_U64,
	PACTON_PRIMTYPE_F32,
	PACTON_PRIMTYPE_F64,
	PACTON_PRIMTYPE__N,
	PACTON_PRIMTYPE_UNKNOWN,
};


#define PACTON_TYPE(endian,size,primtype) ((((endian)&0x1) << 0) | (((size)&0xFF) << 8) | (((primtype)&0xFF) << 16))
#define PACTON_TYPE_BYTEORDER(t) (((t) >> 0) & 0x1)
#define PACTON_TYPE_SIZE(t) (((t) >> 8) & 0xFF)
#define PACTON_TYPE_PRIMTYPE(t) (((t) >> 16) & 0xFF)


char const * pacton_primtype_tostr (enum pacton_primtype type)
{
	switch (type)
	{
	case PACTON_PRIMTYPE_I8: return "I8";
	case PACTON_PRIMTYPE_I16: return "I16";
	case PACTON_PRIMTYPE_I32: return "I32";
	case PACTON_PRIMTYPE_I64: return "I64";
	case PACTON_PRIMTYPE_U8: return "U8";
	case PACTON_PRIMTYPE_U16: return "U16";
	case PACTON_PRIMTYPE_U32: return "U32";
	case PACTON_PRIMTYPE_U64: return "U64";
	case PACTON_PRIMTYPE_F32: return "F32";
	case PACTON_PRIMTYPE_F64: return "F64";
	default:return NULL;
	}
}


enum pacton_primtype pacton_primtype_fromstr (char const * primtype)
{
	assert (primtype);
	if (0){}
	else if (strcmp (primtype, pacton_primtype_tostr(PACTON_PRIMTYPE_I8)) == 0){return PACTON_PRIMTYPE_I8;}
	else if (strcmp (primtype, pacton_primtype_tostr(PACTON_PRIMTYPE_I16)) == 0){return PACTON_PRIMTYPE_I16;}
	else if (strcmp (primtype, pacton_primtype_tostr(PACTON_PRIMTYPE_I32)) == 0){return PACTON_PRIMTYPE_I32;}
	else if (strcmp (primtype, pacton_primtype_tostr(PACTON_PRIMTYPE_I64)) == 0){return PACTON_PRIMTYPE_I64;}
	else if (strcmp (primtype, pacton_primtype_tostr(PACTON_PRIMTYPE_U8)) == 0){return PACTON_PRIMTYPE_U8;}
	else if (strcmp (primtype, pacton_primtype_tostr(PACTON_PRIMTYPE_U16)) == 0){return PACTON_PRIMTYPE_U16;}
	else if (strcmp (primtype, pacton_primtype_tostr(PACTON_PRIMTYPE_U32)) == 0){return PACTON_PRIMTYPE_U32;}
	else if (strcmp (primtype, pacton_primtype_tostr(PACTON_PRIMTYPE_U64)) == 0){return PACTON_PRIMTYPE_U64;}
	else if (strcmp (primtype, pacton_primtype_tostr(PACTON_PRIMTYPE_F32)) == 0){return PACTON_PRIMTYPE_F32;}
	else if (strcmp (primtype, pacton_primtype_tostr(PACTON_PRIMTYPE_F64)) == 0){return PACTON_PRIMTYPE_F64;}
	return PACTON_PRIMTYPE_UNKNOWN;
}


char const * pacton_byteorder_tostr (enum pacton_byteorder order)
{
	switch (order)
	{
	case PACTON_BYTEORDER_BE: return "BE";
	case PACTON_BYTEORDER_LE: return "LE";
	default:return NULL;
	}
}


enum pacton_byteorder pacton_byteorder_fromstr (char const * primtype)
{
	assert (primtype);
	if (0){}
	else if (strcmp (primtype, pacton_byteorder_tostr(PACTON_BYTEORDER_BE)) == 0){return PACTON_BYTEORDER_BE;}
	else if (strcmp (primtype, pacton_byteorder_tostr(PACTON_BYTEORDER_LE)) == 0){return PACTON_BYTEORDER_LE;}
	return PACTON_BYTEORDER_UNKNOWN;
}


enum pacton_type
{
	PACKTON_BE_U1 = PACTON_TYPE(PACTON_BYTEORDER_BE, 1, PACTON_PRIMTYPE_U8),
	PACKTON_BE_U2 = PACTON_TYPE(PACTON_BYTEORDER_BE, 2, PACTON_PRIMTYPE_U8),
	PACKTON_BE_U3 = PACTON_TYPE(PACTON_BYTEORDER_BE, 3, PACTON_PRIMTYPE_U8),
	PACKTON_BE_U4 = PACTON_TYPE(PACTON_BYTEORDER_BE, 4, PACTON_PRIMTYPE_U8),
	PACKTON_BE_U5 = PACTON_TYPE(PACTON_BYTEORDER_BE, 5, PACTON_PRIMTYPE_U8),
	PACKTON_BE_U6 = PACTON_TYPE(PACTON_BYTEORDER_BE, 6, PACTON_PRIMTYPE_U8),
	PACKTON_BE_U7 = PACTON_TYPE(PACTON_BYTEORDER_BE, 7, PACTON_PRIMTYPE_U8),
	PACKTON_BE_U8 = PACTON_TYPE(PACTON_BYTEORDER_BE, 8, PACTON_PRIMTYPE_U8),
	PACKTON_BE_U16 = PACTON_TYPE(PACTON_BYTEORDER_BE, 16, PACTON_PRIMTYPE_U16),
	PACKTON_BE_U24 = PACTON_TYPE(PACTON_BYTEORDER_BE, 24, PACTON_PRIMTYPE_U32),
	PACKTON_BE_U32 = PACTON_TYPE(PACTON_BYTEORDER_BE, 32, PACTON_PRIMTYPE_U32)
};


#define PACTON_VALUE_NAMES0_STEP 32
struct pacton_value
{
	uint32_t n;
	char * names0;
	uint32_t * block;
	uint32_t * bytepos;
	uint32_t * bitpos;
	uint32_t * dim;
	uint32_t * type;
};


#define PACTON_BLOCK_NAMES0_STEP 32
#define PACTON_BLOCK_NAMES1_STEP 16
#define PACTON_BLOCK_DATA_STEP 128
struct pacton_block
{
	uint32_t n;
	char * names0;
	char * names1;
	uint8_t * data;
	uint32_t * data_size; //Size(i) of data(i) element.
	uint32_t * index;
	uint32_t * subindex;
};


void pacton_block_tofile (struct pacton_block * block, char const * filename)
{
	FILE * f = fopen (filename, "w+");

	fprintf (f, "%*.*s ", PACTON_BLOCK_NAMES0_STEP, PACTON_BLOCK_NAMES0_STEP, "Name0");
	fprintf (f, "%*.*s ", PACTON_BLOCK_NAMES1_STEP, PACTON_BLOCK_NAMES1_STEP, "Name1");
	fprintf (f, "%6.6s ", "Size");
	fprintf (f, "%6.6s ", "Index");
	fprintf (f, "%6.6s ", "Subidx");
	fputc ('\n',f);

	for (uint32_t i = 0; i < block->n; ++i)
	{
		char * block_name0 = block->names0 + PACTON_BLOCK_NAMES0_STEP * i;
		char * block_name1 = block->names1 + PACTON_BLOCK_NAMES1_STEP * i;
		uint32_t block_size = block->data_size [i];
		uint32_t block_index = block->index [i];
		uint32_t block_subindex = block->subindex [i];
		fprintf (f, "%*.*s ", PACTON_BLOCK_NAMES0_STEP, PACTON_BLOCK_NAMES0_STEP, block_name0);
		fprintf (f, "%*.*s ", PACTON_BLOCK_NAMES1_STEP, PACTON_BLOCK_NAMES1_STEP, block_name1);
		fprintf (f, "%6i ", (int)block_size);
		fprintf (f, "0x%04x ", block_index);
		fprintf (f, "%6i ", block_subindex);
		fputc ('\n',f);
	}
	fclose (f);
}


void pacton_block_fromfile (struct pacton_block * block, char const * filename)
{
	printf ("\npacton_block_fromfile %s\n", filename);
	char buf [1024];
	FILE * f = fopen (filename, "r");
	if (f == NULL) {perror ("Can not open filename"); goto error;}
	char * r = fgets (buf, (int)sizeof (buf), f);
	if (r == NULL) {perror ("Empty file"); goto error;}
	for (size_t i = 0; i < block->n; ++i)
	{
		r = fgets (buf, (int)sizeof (buf), f);
		if (r == NULL) {break;}
		char * block_name0 = block->names0 + PACTON_BLOCK_NAMES0_STEP * i;
		char * block_name1 = block->names1 + PACTON_BLOCK_NAMES1_STEP* i;
		uint32_t * block_size = block->data_size + i;
		uint32_t * block_index = block->index + i;
		uint32_t * block_subindex = block->subindex + i;
		sscanf (buf, "%s %s %u %x %u", block_name0, block_name1, block_size, block_index, block_subindex);
		printf ("%s %s %u %x %u\n", block_name0, block_name1, *block_size, *block_index, *block_subindex);
	}
error:
	if (f) {fclose (f);}
}


void pacton_value1_tofile (struct pacton_value * value, uint32_t i, FILE * f)
{
	char buf [1024];
	char * name0 = value->names0 + PACTON_VALUE_NAMES0_STEP * i;
	uint32_t block = value->block [i];
	uint32_t bytepos = value->bytepos [i];
	uint32_t bitpos = value->bitpos [i];
	uint32_t dim = value->dim [i];
	uint32_t type = value->type [i];
	snprintf (buf, sizeof (buf), "\"%s\"",  name0);
	fprintf (f, "%*.*s ", PACTON_VALUE_NAMES0_STEP, PACTON_VALUE_NAMES0_STEP, buf);
	fprintf (f, "%6i ", (int)block);
	fprintf (f, "%6i ", (int)bytepos);
	fprintf (f, "%6i ", (int)bitpos);
	fprintf (f, "%6i ", (int)dim);
	fprintf (f, "%6.6s ", pacton_primtype_tostr (PACTON_TYPE_PRIMTYPE (type)));
	fprintf (f, "%6.6s ", pacton_byteorder_tostr (PACTON_TYPE_BYTEORDER (type)));
	fprintf (f, "%6i ", PACTON_TYPE_SIZE (type));
}


void pacton_value_tofile (struct pacton_value * value, FILE * f)
{
	fprintf (f, "%*.*s ", PACTON_VALUE_NAMES0_STEP, PACTON_VALUE_NAMES0_STEP, "Name0");
	fprintf (f, "%6.6s ", "Block");
	fprintf (f, "%6.6s ", "Byte");
	fprintf (f, "%6.6s ", "Bit");
	fprintf (f, "%6.6s ", "Dim");
	fprintf (f, "%6.6s ", "Prim");
	fprintf (f, "%6.6s ", "Order");
	fprintf (f, "%6.6s ", "Size");
	fputc ('\n',f);

	for (uint32_t i = 0; i < value->n; ++i)
	{
		pacton_value1_tofile (value, i, f);
		fputc ('\n',f);
	}
}


void pacton_value_tofilename (struct pacton_value * value, char const * filename)
{
	FILE * f = fopen (filename, "w+");
	assert (f);
	pacton_value_tofile (value, f);
	fclose (f);
}


int pacton_value_scanf
(
char * buf,
char * value_name0,
uint32_t * block,
uint32_t * bytepos,
uint32_t * bitpos,
uint32_t * dim,
uint32_t * type
)
{
	char type_primtype[6];
	char type_byteorder[6];
	uint32_t type_size;
	int r;
	r = sscanf
	(
	buf, " \"%"PACTON_XSTR (PACTON_VALUE_NAMES0_STEP)"[^\"]\" %u %u %u %u %6s %6s %u",
	value_name0, block, bytepos, bitpos, dim, type_primtype, type_byteorder, &type_size
	);
	if (r == 0)
	{
		r = sscanf
		(
		buf, " \"\" %u %u %u %u %6s %6s %u",
		block, bytepos, bitpos, dim, type_primtype, type_byteorder, &type_size
		);
	}
	//printf ("%70.70s READ%i\n", buf, r);
	enum pacton_byteorder byteorder = pacton_byteorder_fromstr (type_byteorder);
	enum pacton_primtype primtype = pacton_primtype_fromstr (type_primtype);
	assert (byteorder != PACTON_BYTEORDER_UNKNOWN);
	assert (primtype != PACTON_PRIMTYPE_UNKNOWN);
	*type = PACTON_TYPE (byteorder, type_size, primtype);
	return r;
}


void pacton_value_fromfile (struct pacton_value * value, char const * filename)
{
	printf ("\npacton_value_fromfile %s\n", filename);
	char buf [1024];
	FILE * f = fopen (filename, "r");
	if (f == NULL) {perror ("Can not open filename"); goto error;}
	char * r = fgets (buf, (int)sizeof (buf), f);
	if (r == NULL) {perror ("Empty file"); goto error;}
	for (uint32_t i = 0; i < value->n; ++i)
	{
		r = fgets (buf, (int)sizeof (buf), f);
		if (r == NULL) {break;}
		char * value_name0 = value->names0 + PACTON_VALUE_NAMES0_STEP * i;
		uint32_t * block = value->block + i;
		uint32_t * bytepos = value->bytepos + i;
		uint32_t * bitpos = value->bitpos + i;
		uint32_t * dim = value->dim + i;
		uint32_t * type = value->type + i;
		//printf ("%s", buf);
		pacton_value_scanf (buf, value_name0, block, bytepos, bitpos, dim, type);
		//pacton_value1_tofile (value, i, stdout);
		putc('\n', stdout);
	}
error:
	if (f) {fclose (f);}
}


void pacton_value_fill (struct pacton_value * record)
{
	snprintf (record->names0 + PACTON_VALUE_NAMES0_STEP * 0, 100, "%s", "Menu Number");
	record->block[0] = 0;
	record->dim[0] = 1;
	record->bytepos[0] = 7;
	record->bitpos[0] = 0;
	record->type[0] = PACTON_TYPE(PACTON_BYTEORDER_LE, 4, PACTON_PRIMTYPE_U8);
	snprintf (record->names0 + PACTON_VALUE_NAMES0_STEP * 1, 100, "%s", "Remote Control Mode");
	record->block[1] = 0;
	record->dim[1] = 1;
	record->bytepos[1] = 7;
	record->bitpos[1] = 4;
	record->type[1] = PACTON_TYPE(PACTON_BYTEORDER_LE, 4, PACTON_PRIMTYPE_U8);
}
