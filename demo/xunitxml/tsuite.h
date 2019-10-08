#pragma once

#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

#include <liblfds711.h>
#include <csc_readmisc.h>
#include <csc_basic.h>
#include <mxml.h>

#include "qasync.h"
#include "threads.h"


#define TSUITE_START_MEMORY_SIZE 128
#define TSUITE_FILENAME_MAXLEN 128
#define TSUITE_FLAG_EMPTY (uint32_t)0x0001



struct tsuite_caseinfo
{
	struct lfds711_stack_element se;
	int id;
	unsigned memory_size;
	char * memory;
	char * filename;
	int rcode;
	mxml_node_t * node;
};


struct tsuite
{
	struct lfds711_stack_state ss;
	size_t tc_count;
	struct tsuite_caseinfo * tc;
	struct qasync * resultq;
	size_t channel_xunit;
	size_t channel_info;
	char const * findcmd;
	char const * workcmd;
	uint32_t flags;
	char const * assertgrep;
};


void tsuite_info (struct tsuite * suite, char const * text)
{
	assert (suite);
	assert (text);
	qasync_add (suite->resultq, suite->channel_info, text);
}


void tsuite_infof (struct tsuite * suite, char const * format, ...)
{
	assert (suite);
	assert (format);
	va_list va;
	va_start (va, format);
	qasync_addfv (suite->resultq, suite->channel_info, format, va);
	va_end (va);
}


char const * rstrstr11 (char const * e, size_t n, char const * substr)
{
	char const * needle = NULL;
	while (n--)
	{
		needle = strstr (e, substr);
		if (needle) {break;}
		e--;
	}
	return needle;
}


/*
<?xml version="1.0" encoding="UTF-8"?>
<testsuite name="nosetests" tests="1" errors="1" failures="0" skip="0">
	<testcase classname="path_to_test_suite.TestSomething"
			  name="test_it" time="0">
		<error type="exceptions.TypeError" message="oops, wrong type">
		Traceback (most recent call last):
		...
		TypeError: oops, wrong type
		</error>
	</testcase>
</testsuite>
*/
void tsuite_runner1 (struct tsuite * suite, struct tsuite_caseinfo * tc)
{
	assert (suite->workcmd);
	assert (suite->assertgrep);
	assert (tc->memory == NULL);
	assert (tc->memory_size == 0);
	char cmd [1024] = {0};
	snprintf (cmd, 1024, suite->workcmd, tc->filename);
	tsuite_infof (suite, "id %i: popen %s\n", tc->id, cmd);

	struct timespec t [2];
	clock_gettime (CLOCK_REALTIME, t + 0);
	FILE * fp = popen (cmd, "r");
	tc->memory_size = TSUITE_START_MEMORY_SIZE;
	tc->memory = csc_readmisc_realloc (fileno (fp), &tc->memory_size);
	tc->memory [tc->memory_size] = '\0';
	tc->rcode = pclose (fp);
	assert (tc->memory);
	clock_gettime (CLOCK_REALTIME, t + 1);
	double spent = (t [1].tv_sec - t [0].tv_sec) + (t [1].tv_nsec + t [1].tv_nsec) / 1000000000.0;

	tsuite_infof (suite, "id %i: rc=%i, size=%iB\n", tc->id, tc->rcode, tc->memory_size);
	tc->node = mxmlNewElement (NULL, "testcase");
	mxmlElementSetAttr (tc->node, "name", tc->filename);
	mxmlElementSetAttrf (tc->node, "time", "%f", spent);
	char const * assertline = rstrstr11 (tc->memory + tc->memory_size, MIN (tc->memory_size, 1000), suite->assertgrep);

	if (assertline == NULL)
	{
		if (tc->rcode)
		{
			mxml_node_t * terror = mxmlNewElement (tc->node, "error");
			mxmlElementSetAttr (terror, "message", "no assert message found");
			mxmlNewCDATA (terror, tc->memory);
		}
		else
		{
			mxmlNewCDATA (tc->node, tc->memory);
		}
	}
	else
	{
		mxml_node_t * terror = NULL;
		if (tc->rcode)
		{
			terror = mxmlNewElement (tc->node, "error");
		}
		else
		{
			terror = mxmlNewElement (tc->node, "failure");
		}
		char amsg [128] = {0};
		char * e = memccpy (amsg, assertline, '\n', 128);
		if (e) {e [-1] = '\0';}
		mxmlElementSetAttr (terror, "message", amsg);
		mxmlNewCDATA (terror, tc->memory);
	}

	free (tc->memory);
	tc->memory = NULL;
	tc->memory_size = 0;
}



void tsuite_runner0 (struct tsuite * suite)
{
	assert (suite);
	struct lfds711_stack_element * se;
	struct tsuite_caseinfo * tc;
	int pop_result = lfds711_stack_pop (&suite->ss, &se);
	if (pop_result == 0)
	{
		suite->flags |= TSUITE_FLAG_EMPTY;
		return;
	}
	tc = LFDS711_STACK_GET_VALUE_FROM_ELEMENT (*se);
	tsuite_runner1 (suite, tc);
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
		suite->tc [i].filename = calloc (TSUITE_FILENAME_MAXLEN, sizeof (char));
		assert (suite->tc [i].filename);
		char * r = fgets (suite->tc [i].filename, TSUITE_FILENAME_MAXLEN, fp);
		if (r == NULL) {break;}
		tsuite_infof (suite, "filename %s\n", suite->tc [i].filename);
		char * e = strchr (suite->tc [i].filename, '\n');
		if (e) {*e = '\0';}
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
	assert (item);
	lfds711_stack_cleanup (&item->ss, NULL);
	for (size_t i = 0; i < item->tc_count; ++i)
	{
		if (item->tc [i].memory)
		{
			assert (item->tc [i].memory_size > 0);
			free (item->tc [i].memory);
			item->tc [i].memory = NULL;
			item->tc [i].memory_size = 0;
		}
	}
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

