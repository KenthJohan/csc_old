#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <unistd.h>

#include <mxml.h>
#include <liblfds711.h>
#include <pthread.h>
#include <errno.h>

#include <csc_readmisc.h>


#include "argparse.h"
#include "biqueue.h"
#include "threads.h"
#include "tsuite.h"




#define THREAD_COUNT 2
#define THREAD_PRIORITY 1
#define THREAD_POLICY SCHED_RR
//#define THREAD_POLICY SCHED_FIFO
//#define THREAD_POLICY SCHED_OTHER

#define DESCRIPTION0 \
"\nThis is test-runner. It will find test and then run them in a thread pool."
#define DESCRIPTION1 \
"\nNo additional description of the program is available in this version."

static struct biqueue textlogger;



static const char *const usage[] =
{
"xunitxml [options] [[--] args]",
"xunitxml [options]",
NULL,
};


void * runner_suite (void * arg)
{
	struct tsuite * suite = arg;
	assert (suite);
	while (1)
	{
		if (suite->flags & TSUITE_FLAG_COMPLETE) {break;}
		sleep (1);
		tsuite_runner0 (suite);
	}
	return NULL;
}


void * runner_textlog (void * arg)
{
	struct biqueue * tlog = arg;
	assert (tlog);
	while (1)
	{
		if (tlog->flags & BIQUEUE_FLAG_COMPLETE) {break;}
		biqueue_runner0 (tlog);
	}
	return NULL;
}


int main (int argc, char const * argv [])
{
	setbuf (stdout, NULL);
	biqueue_init (&textlogger);
	int thread_count = THREAD_COUNT;
	int thread_policy = THREAD_POLICY;
	int thread_priority = THREAD_PRIORITY;
	int caseinfo_count = CASEINFO_COUNT;
	const char *findcmd = NULL;
	const char *logfilename = NULL;
	struct argparse_option options[] =
	{
		OPT_HELP(),
		OPT_GROUP("Basic options"),
		OPT_INTEGER('j', "thread_count", &thread_count, "number of threads", NULL, 0, 0),
		OPT_INTEGER('s', "thread_policy", &thread_policy, "policy of each thread", NULL, 0, 0),
		OPT_INTEGER('n', "thread_priority", &thread_priority, "priority of each thread", NULL, 0, 0),
		OPT_INTEGER('k', "caseinfo_count", &caseinfo_count, "Maximum number of testcases", NULL, 0, 0),
		OPT_STRING('p', "findcmd", &findcmd, "Use linux 'find' command here to find files to test", NULL, 0, 0),
		OPT_STRING('L', "logfilename", &logfilename, "Store log messages in logfilename", NULL, 0, 0),
		OPT_END()
	};
	struct argparse argparse;
	argparse_init(&argparse, options, usage, 0);
	argparse_describe(&argparse, DESCRIPTION0, DESCRIPTION1);
	argc = argparse_parse(&argparse, argc, argv);

	if (findcmd == NULL) {findcmd = CASEINFO_FINDCMD;}
	printf ("\n\nargparse result:\n");
	printf ("options [0].flags %x\n", options [0].flags);
	printf ("thread_count: %d\n", thread_count);
	printf ("thread_policy: %d\n", thread_policy);
	printf ("thread_priority: %d\n", thread_priority);
	printf ("caseinfo_count: %d\n", caseinfo_count);
	printf ("findcmd: %s\n", findcmd);
	printf ("logfilename: %s\n", logfilename);
	if (options [0].flags & OPT_ENABLED)
	{
		exit (0);
	}
	if (logfilename)
	{
		textlogger.fdes = fopen (logfilename, "w+");
	}

	assert (thread_count >= 0);
	assert (caseinfo_count >= 0);
	assert (thread_policy == SCHED_RR || thread_policy == SCHED_FIFO || thread_policy == SCHED_OTHER);




	biqueue_start (&textlogger, 5);

	struct tsuite suite;
	suite.logger = &textlogger;
	suite.tcases_count = (size_t)caseinfo_count;
	suite.findcmd = findcmd;
	tsuite_init (&suite);

	pthread_t thread_textlog;
	textlogger.flags = BIQUEUE_FLAG_RUNNING;
	pthread_create (&thread_textlog, NULL, runner_textlog, &textlogger);
	pthread_t * threads = calloc ((size_t)thread_count, sizeof (pthread_t));
	for (int i = 0; i < thread_count; ++i)
	{
		biqueue_addf(&textlogger, "pthread_create %i of %i\n", i, thread_count-1);
		pthread_create (threads + i, NULL, runner_suite, &suite);
	}
	for (int i = 0; i < thread_count; ++i)
	{
		pthread_join (threads [i], NULL);
	}
	textlogger.flags |= BIQUEUE_FLAG_QUIT_SOFT;
	pthread_join (thread_textlog, NULL);
	return EXIT_SUCCESS;
}

