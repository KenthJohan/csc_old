#pragma once
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <pthread.h>
#include <liblfds711.h>

#define TEXTLOG_MESSAGE_BUFFER_SIZE 1024
#define TEXTLOG_FLAG_QUIT        (uint32_t)0x0001
#define TEXTLOG_FLAG_QUIT_NOWAIT (uint32_t)0x0002
#define TEXTLOG_FLAG_RUNNING     (uint32_t)0x0004

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
	FILE * fdes;
};


void textlog_print (struct textlog_instance * instance, char const * text)
{
	fputs (text, instance->fdes);
}

void textlog_vprintf (struct textlog_instance * instance, char const * fmt, va_list va)
{
	vfprintf (instance->fdes, fmt, va);
}


static void textlog_add (struct textlog_instance * instance, char const * text)
{
	if (instance->flag & TEXTLOG_FLAG_RUNNING)
	{
		struct lfds711_freelist_element * fe;
		int r = lfds711_freelist_pop (&instance->fls_pool, &fe, NULL);
		assert (r == 1);
		struct textlog_message * msg = LFDS711_FREELIST_GET_VALUE_FROM_ELEMENT(*fe);
		memccpy (msg->buffer, text, '\0', TEXTLOG_MESSAGE_BUFFER_SIZE);
		LFDS711_FREELIST_SET_VALUE_IN_ELEMENT (msg->fe, msg);
		lfds711_freelist_push (&instance->fls_comm, &msg->fe, NULL);
	}
	else
	{
		textlog_print (instance, text);
	}
}


static void textlog_addf (struct textlog_instance * instance, char const * fmt, ...)
{
	va_list va;
	va_start (va, fmt);
	if (instance->flag & TEXTLOG_FLAG_RUNNING)
	{
		struct lfds711_freelist_element * fe;
		int r = lfds711_freelist_pop (&instance->fls_pool, &fe, NULL);
		assert (r == 1);
		struct textlog_message * msg = LFDS711_FREELIST_GET_VALUE_FROM_ELEMENT(*fe);
		vsprintf (msg->buffer, fmt, va);
		LFDS711_FREELIST_SET_VALUE_IN_ELEMENT (msg->fe, msg);
		lfds711_freelist_push (&instance->fls_comm, &msg->fe, NULL);
	}
	else
	{
		textlog_vprintf (instance, fmt, va);
	}
	va_end (va);
}


static void * textlog_runner (void * arg)
{
	assert (arg);
	struct textlog_instance * instance = arg;
	instance->flag = TEXTLOG_FLAG_RUNNING;
	while (1)
	{
		if (instance->flag & TEXTLOG_FLAG_QUIT_NOWAIT) {break;}
		struct lfds711_freelist_element * fe;
		int r = lfds711_freelist_pop (&instance->fls_comm, &fe, NULL);
		//Check if list is empty:
		if (r == 0)
		{
			if (instance->flag & TEXTLOG_FLAG_QUIT) {break;}
			sleep (1);
			continue;
		}
		sleep (1);
		struct textlog_message * msg = LFDS711_FREELIST_GET_VALUE_FROM_ELEMENT(*fe);
		textlog_print (instance, msg->buffer);
		LFDS711_FREELIST_SET_VALUE_IN_ELEMENT (msg->fe, msg);
		lfds711_freelist_push (&instance->fls_pool, &msg->fe, NULL);
	}
	instance->flag &= ~TEXTLOG_FLAG_RUNNING;
	lfds711_freelist_cleanup (&instance->fls_pool, NULL);
	lfds711_freelist_cleanup (&instance->fls_comm, NULL);
	textlog_add (instance, "textlog_runner quit\n");
	return NULL;
}


static void textlog_init (struct textlog_instance * instance)
{
	instance->flag = 0;
	instance->messages_count = 0;
	instance->fdes = stdout;
}


static void textlog_start (struct textlog_instance * instance, size_t n)
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




