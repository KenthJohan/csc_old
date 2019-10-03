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
#include "queue_async.h"
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

#define APP_XUNIT_FILENAME "default.xml"
#define APP_FINDCMD "find ../ -name \"*.c\""
#define APP_MSG_COUNT 10
#define APP_MSG_MEM_SIZE 1024


enum app_channel
{
	APP_CHANNEL_STDOUT,
	APP_CHANNEL_XUNITFILE
};


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
	struct queue_async * tlog = arg;
	assert (tlog);
	while (1)
	{
		if (tlog->flags & QUEUE_ASYNC_FLAG_COMPLETE) {break;}
		queue_async_runner0 (tlog);
	}
	return NULL;
}


int main (int argc, char const * argv [])
{
	setbuf (stdout, NULL);

	//Textlogger:
	struct queue_async textlogger = {0};
	textlogger.fdes = calloc (2, sizeof (FILE*));
	textlogger.fdes [APP_CHANNEL_STDOUT] = stdout;

	//Init default program options:
	int thread_count = THREAD_COUNT;
	int thread_policy = THREAD_POLICY;
	int thread_priority = THREAD_PRIORITY;
	int caseinfo_count = CASEINFO_COUNT;
	const char *findcmd = NULL;
	const char *logfilename = NULL;
	const char *xunitfilename = NULL;

	//Define different program options:
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
		OPT_STRING('X', "xunitfilename", &xunitfilename, "Store xunit result in xunitfilename", NULL, 0, 0),
		OPT_END()
	};

	//Parse program options:
	struct argparse argparse;
	argparse_init (&argparse, options, usage, 0);
	argparse_describe (&argparse, DESCRIPTION0, DESCRIPTION1);
	argc = argparse_parse (&argparse, argc, argv);

	//Default:
	if (findcmd == NULL) {findcmd = APP_FINDCMD;}
	if (xunitfilename == NULL) {xunitfilename = APP_XUNIT_FILENAME;}

	//Print selected program options:
	queue_async_add (&textlogger, APP_CHANNEL_STDOUT, "\n\nargparse result:\n");
	queue_async_addf (&textlogger, APP_CHANNEL_STDOUT, "options [0].flags %x\n", options [0].flags);
	queue_async_addf (&textlogger, APP_CHANNEL_STDOUT, "thread_count: %d\n", thread_count);
	queue_async_addf (&textlogger, APP_CHANNEL_STDOUT, "thread_policy: %d\n", thread_policy);
	queue_async_addf (&textlogger, APP_CHANNEL_STDOUT, "thread_priority: %d\n", thread_priority);
	queue_async_addf (&textlogger, APP_CHANNEL_STDOUT, "caseinfo_count: %d\n", caseinfo_count);
	queue_async_addf (&textlogger, APP_CHANNEL_STDOUT, "findcmd: %s\n", findcmd);
	queue_async_addf (&textlogger, APP_CHANNEL_STDOUT, "logfilename: %s\n", logfilename);
	queue_async_addf (&textlogger, APP_CHANNEL_STDOUT, "xunitfilename: %s\n", xunitfilename);

	//Quit when help options is enabled:
	if (options [0].flags & OPT_ENABLED)
	{
		exit (0);
	}

	//Use logfile if (logfilename) option is enabled:
	if (logfilename)
	{
		textlogger.fdes [APP_CHANNEL_STDOUT] = fopen (logfilename, "w+");
		assert (textlogger.fdes [APP_CHANNEL_STDOUT]);
	}

	if (xunitfilename)
	{
		textlogger.fdes [APP_CHANNEL_XUNITFILE] = fopen (xunitfilename, "w+");
		assert (textlogger.fdes [APP_CHANNEL_XUNITFILE]);
	}

	//Guard:
	assert (thread_count >= 0);
	assert (caseinfo_count >= 0);
	assert (thread_policy == SCHED_RR || thread_policy == SCHED_FIFO || thread_policy == SCHED_OTHER);

	//Configure the (tsuite) which will merge test results:
	struct tsuite suite = {0};
	suite.logger = &textlogger;
	suite.tcases_count = (size_t)caseinfo_count;
	suite.findcmd = findcmd;
	suite.channel_xunit = APP_CHANNEL_XUNITFILE;
	suite.channel_logging = APP_CHANNEL_STDOUT;
	suite.flags = 0;
	tsuite_init (&suite);

	//Start (textlogger) thread:
	pthread_t thread_textlog;
	queue_async_init (&textlogger, APP_MSG_COUNT, APP_MSG_MEM_SIZE);
	pthread_create (&thread_textlog, NULL, runner_textlog, &textlogger);

	//Start all worker threads:
	pthread_t * threads = calloc ((size_t)thread_count, sizeof (pthread_t));
	for (int i = 0; i < thread_count; ++i)
	{
		queue_async_addf (&textlogger, APP_CHANNEL_STDOUT, "pthread_create %i of %i\n", i, thread_count-1);
		pthread_create (threads + i, NULL, runner_suite, &suite);
	}
	//sleep (4);
	//Wait for all worker threads to complete:
	for (int i = 0; i < thread_count; ++i)
	{
		pthread_join (threads [i], NULL);
	}

	//When all worker threads are complete quit the textlogger:
	textlogger.flags |= QUEUE_ASYNC_FLAG_QUIT_SOFT;
	pthread_join (thread_textlog, NULL);

	//If we have (logfilename) then we know (fdes) is a file so we can call (fclose):
	if (logfilename && textlogger.fdes [APP_CHANNEL_STDOUT])
	{
		fclose (textlogger.fdes [APP_CHANNEL_STDOUT]);
	}
	if (xunitfilename && textlogger.fdes [APP_CHANNEL_XUNITFILE])
	{
		fclose (textlogger.fdes [APP_CHANNEL_XUNITFILE]);
	}

	return EXIT_SUCCESS;
}

