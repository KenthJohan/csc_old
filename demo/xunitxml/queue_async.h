#pragma once
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <liblfds711.h>

#define QUEUE_ASYNC_MESSAGE_BUFFER_SIZE 1024
#define QUEUE_ASYNC_FLAG_QUIT_SOFT   (uint32_t)0x0001
#define QUEUE_ASYNC_FLAG_QUEUED      (uint32_t)0x0004
#define QUEUE_ASYNC_FLAG_COMPLETE    (uint32_t)0x0008

struct queue_async_msg
{
	struct lfds711_freelist_element fe;
	char * memory;
	size_t memory_size;
	//char buffer [QUEUE_ASYNC_MESSAGE_BUFFER_SIZE];
	size_t channel;
};


struct queue_async
{
	struct lfds711_freelist_state fls_pool;
	struct lfds711_freelist_state fls_comm;
	size_t msg_count;
	struct queue_async_msg * msg;
	uint32_t flags;
	FILE ** fdes;
};


void queue_async_print (struct queue_async * self, size_t channel, char const * text)
{
	fputs (text, self->fdes [channel]);
}

void queue_async_vprintf (struct queue_async * self, size_t channel, char const * fmt, va_list va)
{
	vfprintf (self->fdes [channel], fmt, va);
}


static void queue_async_add (struct queue_async * self, size_t channel, char const * text)
{
	if (self->flags & QUEUE_ASYNC_FLAG_QUEUED)
	{
		struct lfds711_freelist_element * fe;
		int r = lfds711_freelist_pop (&self->fls_pool, &fe, NULL);
		assert (r == 1);
		struct queue_async_msg * msg = LFDS711_FREELIST_GET_VALUE_FROM_ELEMENT(*fe);
		memccpy (msg->memory, text, '\0', msg->memory_size);
		msg->channel = channel;
		LFDS711_FREELIST_SET_VALUE_IN_ELEMENT (msg->fe, msg);
		lfds711_freelist_push (&self->fls_comm, &msg->fe, NULL);
	}
	else
	{
		queue_async_print (self, channel, text);
	}
}


static void queue_async_addf (struct queue_async * bq, size_t channel, char const * fmt, ...)
{
	va_list va;
	va_start (va, fmt);
	if (bq->flags & QUEUE_ASYNC_FLAG_QUEUED)
	{
		struct lfds711_freelist_element * fe;
		int r = lfds711_freelist_pop (&bq->fls_pool, &fe, NULL);
		assert (r == 1);
		struct queue_async_msg * msg = LFDS711_FREELIST_GET_VALUE_FROM_ELEMENT(*fe);
		vsnprintf (msg->memory, msg->memory_size, fmt, va);
		msg->channel = channel;
		LFDS711_FREELIST_SET_VALUE_IN_ELEMENT (msg->fe, msg);
		lfds711_freelist_push (&bq->fls_comm, &msg->fe, NULL);
	}
	else
	{
		queue_async_vprintf (bq, channel, fmt, va);
	}
	va_end (va);
}


void queue_async_runner0 (struct queue_async * self)
{
	assert (self);
	assert (self->flags & QUEUE_ASYNC_FLAG_QUEUED);
	struct lfds711_freelist_element * fe;
	int r = lfds711_freelist_pop (&self->fls_comm, &fe, NULL);
	//printf ("lfds711_freelist_pop %i\n", r);
	//Check if list is empty:
	if (r == 0)
	{
		if (self->flags & QUEUE_ASYNC_FLAG_QUIT_SOFT)
		{
			self->flags |= QUEUE_ASYNC_FLAG_COMPLETE;
		}
	}
	else
	{
		struct queue_async_msg * msg = LFDS711_FREELIST_GET_VALUE_FROM_ELEMENT(*fe);
		queue_async_print (self, msg->channel, msg->memory);
		LFDS711_FREELIST_SET_VALUE_IN_ELEMENT (msg->fe, msg);
		lfds711_freelist_push (&self->fls_pool, &msg->fe, NULL);
	}
}


void queue_async_cleanup (struct queue_async * self)
{
	assert (self);
	assert (self->msg);
	lfds711_freelist_cleanup (&self->fls_pool, NULL);
	lfds711_freelist_cleanup (&self->fls_comm, NULL);
	for (size_t i = 0; i < self->msg_count ; ++i)
	{
		assert (self->msg [i].memory);
		free (self->msg [i].memory);
	}
	free (self->msg);
}


static void queue_async_init (struct queue_async * self, size_t msg_count, size_t msg_mem_size)
{
	self->flags = QUEUE_ASYNC_FLAG_QUEUED;
	lfds711_freelist_init_valid_on_current_logical_core (&self->fls_pool, NULL, 0, NULL);
	lfds711_freelist_init_valid_on_current_logical_core (&self->fls_comm, NULL, 0, NULL);
	self->msg = calloc (msg_count, sizeof (struct queue_async_msg));
	self->msg_count = msg_count;
	for (size_t i = 0; i < msg_count ; ++i)
	{
		self->msg [i].memory = calloc (msg_mem_size, sizeof (char));
		assert (self->msg [i].memory);
		self->msg [i].memory_size = msg_mem_size;
		LFDS711_FREELIST_SET_VALUE_IN_ELEMENT (self->msg [i].fe, &self->msg [i]);
		lfds711_freelist_push (&self->fls_pool, &self->msg [i].fe, NULL);
	}
}




