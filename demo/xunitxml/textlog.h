#pragma once
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <pthread.h>
#include <liblfds711.h>

#define TEXTLOG_MESSAGE_BUFFER_SIZE 1024
#define TEXTLOG_FLAG_QUIT 0x0002

struct textlog_message
{
	struct lfds711_freelist_element fe;
	char buffer [TEXTLOG_MESSAGE_BUFFER_SIZE];
};


struct textlog_instance
{
	struct lfds711_freelist_state fls_pool;
	struct lfds711_freelist_state fls_comm;
	size_t messages_count;
	struct textlog_message * messages;
	pthread_t thread;
	uint32_t flag;
};


static void * textlog_runner (void * arg)
{
	assert (arg);
	struct textlog_instance * instance = arg;
	while (1)
	{
		if (instance->flag & TEXTLOG_FLAG_QUIT) {break;}
		struct lfds711_freelist_element * fe;
		int r = lfds711_freelist_pop (&instance->fls_comm, &fe, NULL);
		if (r == 0)
		{
			sleep (1);
			continue;
		}
		struct textlog_message * msg = LFDS711_FREELIST_GET_VALUE_FROM_ELEMENT(*fe);
		fprintf (stdout, "%s", msg->buffer);
		LFDS711_FREELIST_SET_VALUE_IN_ELEMENT (msg->fe, msg);
		lfds711_freelist_push (&instance->fls_pool, &msg->fe, NULL);
	}
	lfds711_freelist_cleanup (&instance->fls_pool, NULL);
	lfds711_freelist_cleanup (&instance->fls_comm, NULL);
	printf ("textlog_runner quit");
	return NULL;
}


static void textlog_init (struct textlog_instance * instance, size_t n)
{
	lfds711_freelist_init_valid_on_current_logical_core (&instance->fls_pool, NULL, 0, NULL);
	lfds711_freelist_init_valid_on_current_logical_core (&instance->fls_comm, NULL, 0, NULL);
	instance->messages = calloc (n, sizeof (struct textlog_message));
	instance->messages_count = n;
	for (size_t i = 0; i < n ; ++i)
	{
		LFDS711_FREELIST_SET_VALUE_IN_ELEMENT (instance->messages [i].fe, &instance->messages [i]);
		lfds711_freelist_push (&instance->fls_pool, &instance->messages [i].fe, NULL);
	}
	pthread_create (&instance->thread, NULL, textlog_runner, instance);
}


static void textlog_add (struct textlog_instance * instance, char const * text)
{
	struct lfds711_freelist_element * fe;
	int r = lfds711_freelist_pop (&instance->fls_pool, &fe, NULL);
	assert (r == 1);
	struct textlog_message * msg = LFDS711_FREELIST_GET_VALUE_FROM_ELEMENT(*fe);
	memccpy (msg->buffer, text, '\0', TEXTLOG_MESSAGE_BUFFER_SIZE);
	LFDS711_FREELIST_SET_VALUE_IN_ELEMENT (msg->fe, msg);
	lfds711_freelist_push (&instance->fls_comm, &msg->fe, NULL);
}


static void textlog_addf (struct textlog_instance * instance, char const * fmt, ...)
{
	struct lfds711_freelist_element * fe;
	int r = lfds711_freelist_pop (&instance->fls_pool, &fe, NULL);
	assert (r == 1);
	va_list list;
	va_start (list, fmt);
	struct textlog_message * msg = LFDS711_FREELIST_GET_VALUE_FROM_ELEMENT(*fe);
	vsprintf (msg->buffer, fmt, list);
	va_end (list);
	LFDS711_FREELIST_SET_VALUE_IN_ELEMENT (msg->fe, msg);
	lfds711_freelist_push (&instance->fls_comm, &msg->fe, NULL);
}




