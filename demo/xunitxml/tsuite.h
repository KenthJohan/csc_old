#pragma once

#include <liblfds711.h>
#include <csc_readmisc.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include "textlog.h"
#include "threads.h"


#define CASEINFO_COUNT 11
#define CASEINFO_START_MEMORY_SIZE 128
#define CASEINFO_FILENAME_MAXLEN 128
#define CASEINFO_FINDCMD "find ../ -name \"*.c\""




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
	size_t threads_count;
	pthread_t * threads;
	struct textlog_instance * logger;
	char const * findcmd;
	int thread_policy;
	int thread_priority;
};


void * tsuite_caseinfo_runner (void * arg)
{
	int r;
	assert (arg);
	struct tsuite * suite = arg;
	while (1)
	{
		struct lfds711_stack_element * se;
		struct tsuite_caseinfo * cinfo;
		r = lfds711_stack_pop (&suite->ss, &se);
		if (r == 0) {break;}
		cinfo = LFDS711_STACK_GET_VALUE_FROM_ELEMENT (*se);
		textlog_addf (suite->logger, "lfds711_stack_pop %i\n", cinfo->id);
		sleep (rand() % 2); //Sleep just for simulation
		FILE * fp = popen ("netstat", "r");
		cinfo->memory_size = CASEINFO_START_MEMORY_SIZE;
		cinfo->memory = csc_readmisc_realloc (fileno (fp), &cinfo->memory_size);
		r = pclose (fp);
		textlog_addf (suite->logger, "caseinfo_popen : pclose %i\n", r);
		assert (cinfo->memory);
		textlog_addf (suite->logger, "===========\nid %i. n %i. pclose %i\n%.*s\n", cinfo->id, cinfo->memory_size, r, 64, cinfo->memory);
	}
	return NULL;
}

void tsuite_start (struct tsuite * suite)
{
	suite->tcases = malloc (sizeof(struct tsuite_caseinfo) * suite->tcases_count);
	suite->threads = malloc (sizeof (pthread_t) * suite->threads_count);
	lfds711_stack_init_valid_on_current_logical_core (&suite->ss, NULL);
	textlog_addf (suite->logger, "findcmd %s\n", suite->findcmd);
	FILE * fp = popen (suite->findcmd, "r");
	assert (fp);
	//Populate filenames:
	for (size_t i = 0; i < suite->tcases_count; ++i)
	{
		suite->tcases [i].filename = malloc (CASEINFO_FILENAME_MAXLEN);
		assert (suite->tcases [i].filename);
		char * r = fgets (suite->tcases [i].filename, CASEINFO_FILENAME_MAXLEN, fp);
		if (r == NULL) {break;}
		textlog_addf (suite->logger, "filename %s\n", suite->tcases [i].filename);
	}
	int r = pclose (fp);
	textlog_addf (suite->logger, "pclose %i\n", r);
	//Populate stack for proccessing later:
	for (size_t i = 0; i < suite->tcases_count; ++i)
	{
		suite->tcases [i].id = (int)i;
		LFDS711_STACK_SET_VALUE_IN_ELEMENT (suite->tcases [i].se, suite->tcases + i);
		lfds711_stack_push (&suite->ss, &suite->tcases [i].se);
	}
	threads_create (suite->threads, suite->threads_count, tsuite_caseinfo_runner, suite, suite->thread_policy, suite->thread_priority);
}


void tsuite_cleanup (struct tsuite * item)
{
	lfds711_stack_cleanup (&item->ss, NULL);
	free (item->tcases);
	free (item->threads);
}
