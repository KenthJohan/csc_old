#pragma once

#include <liblfds711.h>
#include <csc_readmisc.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include "queue_async.h"
#include "threads.h"


#define CASEINFO_COUNT 11
#define CASEINFO_START_MEMORY_SIZE 128
#define CASEINFO_FILENAME_MAXLEN 128


#define TSUITE_FLAG_COMPLETE 0x0001

struct tsuite_caseinfo
{
	struct lfds711_stack_element se;
	int id;
	unsigned memory_size;
	char * memory;
	char * filename;
	int rcode;
};


struct tsuite
{
	struct lfds711_stack_state ss;
	size_t tc_count;
	struct tsuite_caseinfo * tc;
	struct queue_async * resultq;
	size_t channel_xunit;
	size_t channel_info;
	char const * findcmd;
	char const * workcmd;
	uint32_t flags;
};


void tsuite_info (struct tsuite * suite, char const * text)
{
	queue_async_add (suite->resultq, suite->channel_info, text);
}

void tsuite_infof (struct tsuite * suite, char const * format, ...)
{
	va_list va;
	va_start (va, format);
	queue_async_addfv (suite->resultq, suite->channel_info, format, va);
	va_end (va);
}

void tsuite_tc_result (struct tsuite * suite, struct tsuite_caseinfo * tc)
{
	queue_async_addf (suite->resultq, suite->channel_xunit, "===========\nid %i. n %i. pclose %i\n%.*s\n", tc->id, tc->memory_size, tc->rcode, 64, tc->memory);
}


void tsuite_runner0 (struct tsuite * suite)
{
	struct lfds711_stack_element * se;
	struct tsuite_caseinfo * tc;
	int pop_result = lfds711_stack_pop (&suite->ss, &se);
	if (pop_result == 0)
	{
		suite->flags |= TSUITE_FLAG_COMPLETE;
		return;
	}
	tc = LFDS711_STACK_GET_VALUE_FROM_ELEMENT (*se);
	tsuite_infof (suite, "lfds711_stack_pop %i\n", tc->id);
	sleep (rand() % 2); //Sleep just for simulation
	assert (suite->workcmd);
	FILE * fp = popen (suite->workcmd, "r");
	tc->memory_size = CASEINFO_START_MEMORY_SIZE;
	tc->memory = csc_readmisc_realloc (fileno (fp), &tc->memory_size);
	tc->rcode = pclose (fp);
	assert (tc->memory);
	tsuite_tc_result (suite, tc);
}


void tsuite_init (struct tsuite * suite)
{
	assert (suite);
	assert (suite->resultq);
	suite->tc = calloc (suite->tc_count, sizeof(struct tsuite_caseinfo));
	assert (suite->tc);
	lfds711_stack_init_valid_on_current_logical_core (&suite->ss, NULL);
	tsuite_infof (suite, "findcmd %s\n", suite->findcmd);
	FILE * fp = popen (suite->findcmd, "r");
	assert (fp);
	//Populate filenames:
	size_t i;
	for (i = 0; i < suite->tc_count; ++i)
	{
		suite->tc [i].filename = malloc (CASEINFO_FILENAME_MAXLEN);
		assert (suite->tc [i].filename);
		char * r = fgets (suite->tc [i].filename, CASEINFO_FILENAME_MAXLEN, fp);
		if (r == NULL) {break;}
		tsuite_infof (suite, "filename %s\n", suite->tc [i].filename);
	}
	suite->tc_count = i;
	void * mem = realloc (suite->tc, suite->tc_count * sizeof(struct tsuite_caseinfo));
	if (mem)
	{
		suite->tc = mem;
	}
	tsuite_infof (suite, "Find complete %i testcases found\n", suite->tc_count);
	int r = pclose (fp);
	tsuite_infof (suite, "pclose %i\n", r);
	//Populate stack for proccessing later:
	for (size_t i = 0; i < suite->tc_count; ++i)
	{
		suite->tc [i].id = (int)i;
		LFDS711_STACK_SET_VALUE_IN_ELEMENT (suite->tc [i].se, suite->tc + i);
		lfds711_stack_push (&suite->ss, &suite->tc [i].se);
	}
}


void tsuite_cleanup (struct tsuite * item)
{
	lfds711_stack_cleanup (&item->ss, NULL);
	if (item->tc)
	{
		free (item->tc);
		item->tc = NULL;
		item->tc_count = 0;
	}
	else
	{
		assert (item->tc_count == 0);
	}
}
