#pragma once
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <liblfds711.h>

//Indicates that the queue will be complete after its empty:
#define QASYNC_FLAG_QUIT   (uint32_t)0x0001

//Indicates that its ok to add messages from threads:
#define QASYNC_FLAG_MULTITHREAD_SUPPORTED   (uint32_t)0x0004

//Indicates
#define QASYNC_FLAG_EMPTY       (uint32_t)0x0010

struct qasync_msg
{
	struct lfds711_freelist_element fe;
	char * memory;
	size_t memory_size;
	//char buffer [QASYNC_MESSAGE_BUFFER_SIZE];
	size_t channel;
};


struct qasync
{
	//Message pool stores all messages that is available to use:
	struct lfds711_freelist_state fls_pool;
	//Message comm stores all messages that is queued to process:
	struct lfds711_freelist_state fls_comm;

	size_t msg_count;
	struct qasync_msg * msg;

	uint32_t flags;

	//Use multiple destinations (note double pointer),
	//a message stores a channel index indicting which (fdes) to use:
	FILE ** fdes;
};


void qasync_print (struct qasync * self, size_t channel, char const * text)
{
	assert (self);
	assert (text);
	fputs (text, self->fdes [channel]);
}


void qasync_vprintf (struct qasync * self, size_t channel, char const * fmt, va_list va)
{
	assert (self);
	assert (fmt);
	vfprintf (self->fdes [channel], fmt, va);
}


static void qasync_add (struct qasync * self, size_t channel, char const * text)
{
	assert (self);
	assert (text);
	if (self->flags & QASYNC_FLAG_MULTITHREAD_SUPPORTED)
	{
		struct lfds711_freelist_element * fe;
		int r = lfds711_freelist_pop (&self->fls_pool, &fe, NULL);
		assert (r == 1);
		struct qasync_msg * msg = LFDS711_FREELIST_GET_VALUE_FROM_ELEMENT(*fe);
		memccpy (msg->memory, text, '\0', msg->memory_size);
		msg->channel = channel;
		LFDS711_FREELIST_SET_VALUE_IN_ELEMENT (msg->fe, msg);
		lfds711_freelist_push (&self->fls_comm, &msg->fe, NULL);
	}
	else
	{
		qasync_print (self, channel, text);
	}
}


static void qasync_addfv (struct qasync * self, size_t channel, char const * format, va_list va)
{
	assert (self);
	assert (format);
	if (self->flags & QASYNC_FLAG_MULTITHREAD_SUPPORTED)
	{
		struct lfds711_freelist_element * fe;
		int r = lfds711_freelist_pop (&self->fls_pool, &fe, NULL);
		assert (r == 1);
		struct qasync_msg * msg = LFDS711_FREELIST_GET_VALUE_FROM_ELEMENT(*fe);
		vsnprintf (msg->memory, msg->memory_size, format, va);
		msg->channel = channel;
		LFDS711_FREELIST_SET_VALUE_IN_ELEMENT (msg->fe, msg);
		lfds711_freelist_push (&self->fls_comm, &msg->fe, NULL);
	}
	else
	{
		qasync_vprintf (self, channel, format, va);
	}
}


static void qasync_addf (struct qasync * self, size_t channel, char const * format, ...)
{
	assert (self);
	assert (format);
	va_list va;
	va_start (va, format);
	qasync_addfv (self, channel, format, va);
	va_end (va);
}


void qasync_runner0 (struct qasync * self)
{
	assert (self);
	assert (self->flags & QASYNC_FLAG_MULTITHREAD_SUPPORTED);
	struct lfds711_freelist_element * fe;
	int r = lfds711_freelist_pop (&self->fls_comm, &fe, NULL);
	//printf ("lfds711_freelist_pop %i\n", r);
	//Check if list is empty:
	if (r == 0)
	{
		self->flags |= QASYNC_FLAG_EMPTY;
	}
	else
	{
		self->flags &= ~QASYNC_FLAG_EMPTY;
		struct qasync_msg * msg = LFDS711_FREELIST_GET_VALUE_FROM_ELEMENT(*fe);
		qasync_print (self, msg->channel, msg->memory);
		LFDS711_FREELIST_SET_VALUE_IN_ELEMENT (msg->fe, msg);
		lfds711_freelist_push (&self->fls_pool, &msg->fe, NULL);
	}
}


void qasync_cleanup (struct qasync * self)
{
	assert (self);
	assert (self->msg);
	lfds711_freelist_cleanup (&self->fls_pool, NULL);
	lfds711_freelist_cleanup (&self->fls_comm, NULL);
	if (self->msg)
	{
		assert (self->msg_count > 0);
		for (size_t i = 0; i < self->msg_count ; ++i)
		{
			assert (self->msg [i].memory);
			free (self->msg [i].memory);
		}
		free (self->msg);
	}
}


static void qasync_init (struct qasync * self, size_t msg_mem_size)
{
	assert (self);
	lfds711_freelist_init_valid_on_current_logical_core (&self->fls_pool, NULL, 0, NULL);
	lfds711_freelist_init_valid_on_current_logical_core (&self->fls_comm, NULL, 0, NULL);
	//Allocate all messages:
	self->msg = calloc (self->msg_count, sizeof (struct qasync_msg));
	assert (self->msg);
	for (size_t i = 0; i < self->msg_count ; ++i)
	{
		//Allocate each message memory:
		self->msg [i].memory = calloc (msg_mem_size, sizeof (char));
		assert (self->msg [i].memory);
		self->msg [i].memory_size = msg_mem_size;
		//Add all allocated messages to the message pool.
		LFDS711_FREELIST_SET_VALUE_IN_ELEMENT (self->msg [i].fe, &self->msg [i]);
		lfds711_freelist_push (&self->fls_pool, &self->msg [i].fe, NULL);
	}
	//Now it should be ok to start using threads:
	self->flags = QASYNC_FLAG_MULTITHREAD_SUPPORTED;
}




