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
};


struct tsuite
{
	struct lfds711_stack_state ss;
	size_t tcases_count;
	struct tsuite_caseinfo * tcases;
	struct queue_async * logger;
	size_t channel_xunit;
	size_t channel_logging;
	char const * findcmd;
	uint32_t flags;
};


void tsuite_runner0 (struct tsuite * suite)
{
	struct lfds711_stack_element * se;
	struct tsuite_caseinfo * cinfo;
	int pop_result = lfds711_stack_pop (&suite->ss, &se);
	if (pop_result == 0)
	{
		suite->flags |= TSUITE_FLAG_COMPLETE;
		return;
	}
	cinfo = LFDS711_STACK_GET_VALUE_FROM_ELEMENT (*se);
	queue_async_addf (suite->logger, suite->channel_logging, "lfds711_stack_pop %i\n", cinfo->id);
	sleep (rand() % 2); //Sleep just for simulation
	FILE * fp = popen ("netstat", "r");
	cinfo->memory_size = CASEINFO_START_MEMORY_SIZE;
	cinfo->memory = csc_readmisc_realloc (fileno (fp), &cinfo->memory_size);
	int pcloser = pclose (fp);
	queue_async_addf (suite->logger, suite->channel_xunit, "caseinfo_popen : pclose %i\n", pcloser);
	assert (cinfo->memory);
	queue_async_addf (suite->logger, suite->channel_xunit, "===========\nid %i. n %i. pclose %i\n%.*s\n", cinfo->id, cinfo->memory_size, pcloser, 64, cinfo->memory);
}


void * tsuite_runner (void * arg)
{
	assert (arg);
	struct tsuite * suite = arg;
	while (1)
	{
		tsuite_runner0 (suite);
	}
	return NULL;
}

void tsuite_init (struct tsuite * suite)
{
	suite->tcases = malloc (sizeof(struct tsuite_caseinfo) * suite->tcases_count);
	lfds711_stack_init_valid_on_current_logical_core (&suite->ss, NULL);
	queue_async_addf (suite->logger, suite->channel_logging, "findcmd %s\n", suite->findcmd);
	FILE * fp = popen (suite->findcmd, "r");
	assert (fp);
	//Populate filenames:
	for (size_t i = 0; i < suite->tcases_count; ++i)
	{
		suite->tcases [i].filename = malloc (CASEINFO_FILENAME_MAXLEN);
		assert (suite->tcases [i].filename);
		char * r = fgets (suite->tcases [i].filename, CASEINFO_FILENAME_MAXLEN, fp);
		if (r == NULL) {break;}
		queue_async_addf (suite->logger, suite->channel_logging, "filename %s\n", suite->tcases [i].filename);
	}
	int r = pclose (fp);
	queue_async_addf (suite->logger, suite->channel_logging, "pclose %i\n", r);
	//Populate stack for proccessing later:
	for (size_t i = 0; i < suite->tcases_count; ++i)
	{
		suite->tcases [i].id = (int)i;
		LFDS711_STACK_SET_VALUE_IN_ELEMENT (suite->tcases [i].se, suite->tcases + i);
		lfds711_stack_push (&suite->ss, &suite->tcases [i].se);
	}
}


void tsuite_cleanup (struct tsuite * item)
{
	lfds711_stack_cleanup (&item->ss, NULL);
	free (item->tcases);
}
