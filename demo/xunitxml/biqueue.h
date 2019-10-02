#pragma once
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <liblfds711.h>

#define BIQUEUE_MESSAGE_BUFFER_SIZE 1024
#define BIQUEUE_FLAG_QUIT_SOFT   (uint32_t)0x0001
#define BIQUEUE_FLAG_RUNNING     (uint32_t)0x0004
#define BIQUEUE_FLAG_COMPLETE    (uint32_t)0x0008

struct biqueue_msg
{
	struct lfds711_freelist_element fe;
	char buffer [BIQUEUE_MESSAGE_BUFFER_SIZE];
};


struct biqueue
{
	struct lfds711_freelist_state fls_pool;
	struct lfds711_freelist_state fls_comm;
	size_t messages_count;
	struct biqueue_msg * messages;
	uint32_t flags;
	FILE * fdes;
};


void biqueue_print (struct biqueue * self, char const * text)
{
	fputs (text, self->fdes);
}

void biqueue_vprintf (struct biqueue * self, char const * fmt, va_list va)
{
	vfprintf (self->fdes, fmt, va);
}


static void biqueue_add (struct biqueue * self, char const * text)
{
	if (self->flags & BIQUEUE_FLAG_RUNNING)
	{
		struct lfds711_freelist_element * fe;
		int r = lfds711_freelist_pop (&self->fls_pool, &fe, NULL);
		assert (r == 1);
		struct biqueue_msg * msg = LFDS711_FREELIST_GET_VALUE_FROM_ELEMENT(*fe);
		memccpy (msg->buffer, text, '\0', BIQUEUE_MESSAGE_BUFFER_SIZE);
		LFDS711_FREELIST_SET_VALUE_IN_ELEMENT (msg->fe, msg);
		lfds711_freelist_push (&self->fls_comm, &msg->fe, NULL);
	}
	else
	{
		biqueue_print (self, text);
	}
}


static void biqueue_addf (struct biqueue * bq, char const * fmt, ...)
{
	va_list va;
	va_start (va, fmt);
	if (bq->flags & BIQUEUE_FLAG_RUNNING)
	{
		struct lfds711_freelist_element * fe;
		int r = lfds711_freelist_pop (&bq->fls_pool, &fe, NULL);
		assert (r == 1);
		struct biqueue_msg * msg = LFDS711_FREELIST_GET_VALUE_FROM_ELEMENT(*fe);
		vsprintf (msg->buffer, fmt, va);
		LFDS711_FREELIST_SET_VALUE_IN_ELEMENT (msg->fe, msg);
		lfds711_freelist_push (&bq->fls_comm, &msg->fe, NULL);
	}
	else
	{
		biqueue_vprintf (bq, fmt, va);
	}
	va_end (va);
}


void biqueue_runner0 (struct biqueue * self)
{
	assert (self);
	assert (self->flags & BIQUEUE_FLAG_RUNNING);
	struct lfds711_freelist_element * fe;
	int r = lfds711_freelist_pop (&self->fls_comm, &fe, NULL);
	//printf ("lfds711_freelist_pop %i\n", r);
	//Check if list is empty:
	if (r == 0)
	{
		if (self->flags & BIQUEUE_FLAG_QUIT_SOFT)
		{
			self->flags |= BIQUEUE_FLAG_COMPLETE;
		}
	}
	else
	{
		struct biqueue_msg * msg = LFDS711_FREELIST_GET_VALUE_FROM_ELEMENT(*fe);
		biqueue_print (self, msg->buffer);
		LFDS711_FREELIST_SET_VALUE_IN_ELEMENT (msg->fe, msg);
		lfds711_freelist_push (&self->fls_pool, &msg->fe, NULL);
	}
}


void biqueue_cleanup (struct biqueue * self)
{
	lfds711_freelist_cleanup (&self->fls_pool, NULL);
	lfds711_freelist_cleanup (&self->fls_comm, NULL);
	free (self->messages);
}


static void biqueue_init (struct biqueue * self)
{
	self->flags = 0;
	self->messages_count = 0;
	self->fdes = stdout;
}


static void biqueue_start (struct biqueue * self, size_t n)
{
	lfds711_freelist_init_valid_on_current_logical_core (&self->fls_pool, NULL, 0, NULL);
	lfds711_freelist_init_valid_on_current_logical_core (&self->fls_comm, NULL, 0, NULL);
	self->messages = calloc (n, sizeof (struct biqueue_msg));
	self->messages_count = n;
	for (size_t i = 0; i < n ; ++i)
	{
		LFDS711_FREELIST_SET_VALUE_IN_ELEMENT (self->messages [i].fe, &self->messages [i]);
		lfds711_freelist_push (&self->fls_pool, &self->messages [i].fe, NULL);
	}
}




