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
#include "textlog.h"
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

static struct textlog_instance textlogger;



static const char *const usage[] =
{
"xunitxml [options] [[--] args]",
"xunitxml [options]",
NULL,
};

int main (int argc, char const * argv [])
{
	setbuf (stdout, NULL);
	textlog_init (&textlogger);
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

	textlog_start (&textlogger, 20);

	struct tsuite suite;
	suite.logger = &textlogger;
	suite.tcases_count = (size_t)caseinfo_count;
	suite.threads_count = (size_t)thread_count;
	suite.findcmd = findcmd;
	suite.thread_policy = thread_policy;
	suite.thread_priority = thread_priority;
	tsuite_start (&suite);
	threads_join (suite.threads, suite.threads_count);

	printf ("TEXTLOG_FLAG_QUIT\n");
	textlogger.flag = TEXTLOG_FLAG_QUIT;
	pthread_join (textlogger.thread, NULL);

	return EXIT_SUCCESS;
}

