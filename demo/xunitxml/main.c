#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <unistd.h>
#include <time.h>

#include <mxml.h>
#include <liblfds711.h>
#include <pthread.h>
#include <errno.h>

#include <csc_readmisc.h>


#include "argparse.h"
#include "queue_async.h"
#include "threads.h"
#include "tsuite.h"







#define DESCRIPTION0 \
"\nThis is test-runner. It will find test and then run them in a thread pool."
#define DESCRIPTION1 \
"\nNo additional description of the program is available in this version."

#define APP_XUNIT_FILENAME "default.xml"
#define APP_FINDCMD "find ../ -name \"*.c\""
#define APP_MSG_COUNT 100
#define APP_MSG_MEM_SIZE 1024
#define APP_WORKCMD "netstat"
#define APP_THREAD_COUNT 2

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

static pthread_cond_t condition;

void * runner_suite (void * arg)
{
	struct tsuite * suite = arg;
	assert (suite);
	while (1)
	{
		if (suite->flags & TSUITE_FLAG_COMPLETE) {break;}
		sleep (1);
		tsuite_runner0 (suite);
		pthread_cond_signal (&condition);
		//queue_async_add(suite->logger, APP_CHANNEL_STDOUT, "Hello\n");
	}
	return NULL;
}



void * runner_textlog (void * arg)
{
	pthread_mutex_t mutex;
	pthread_mutex_init (&mutex, NULL);
	struct queue_async * tlog = arg;
	assert (tlog);
	while (1)
	{
		queue_async_runner0 (tlog);
		if (tlog->flags & QUEUE_ASYNC_FLAG_EMPTY)
		{
			struct timespec ts;
			clock_gettime(CLOCK_REALTIME, &ts);
			ts.tv_sec += 1;
			pthread_mutex_lock (&mutex);
			pthread_cond_timedwait (&condition, &mutex, &ts);
			pthread_mutex_unlock (&mutex);
		}
		if (tlog->flags & QUEUE_ASYNC_FLAG_COMPLETE) {break;}
	}
	return NULL;
}


int main (int argc, char const * argv [])
{
	setbuf (stdout, NULL);
	pthread_cond_init (&condition, NULL);

	//Use (appresult) to store any result from multiple threads or single thread:
	//It is designed to be thread safe:
	struct queue_async appresult = {0};
	appresult.fdes = calloc (2, sizeof (FILE*));
	appresult.fdes [APP_CHANNEL_STDOUT] = stdout;
	appresult.fdes [APP_CHANNEL_XUNITFILE] = NULL; //The destination will be defined later.

	//Init default program options:
	int thread_count = APP_THREAD_COUNT;
	int caseinfo_count = CASEINFO_COUNT;
	const char *findcmd = NULL;
	const char *workcmd = NULL;
	const char *logfilename = NULL;
	const char *xunitfilename = NULL;

	//Define different program options:
	struct argparse_option options[] =
	{
		OPT_HELP(),
		OPT_GROUP("Basic options"),
		OPT_INTEGER('j', "thread_count", &thread_count, "number of threads", NULL, 0, 0),
		OPT_INTEGER('k', "caseinfo_count", &caseinfo_count, "Maximum number of testcases", NULL, 0, 0),
		OPT_STRING('f', "findcmd", &findcmd, "Use linux 'find' command here to find files to test", NULL, 0, 0),
		OPT_STRING('w', "jobcmd", &workcmd, "The command which is spread out among threads.", NULL, 0, 0),
		OPT_STRING('l', "logfilename", &logfilename, "Store log messages in logfilename", NULL, 0, 0),
		OPT_STRING('x', "xunitfilename", &xunitfilename, "Store xunit result in xunitfilename", NULL, 0, 0),
		OPT_END()
	};

	//Parse program options:
	struct argparse argparse;
	argparse_init (&argparse, options, usage, 0);
	argparse_describe (&argparse, DESCRIPTION0, DESCRIPTION1);
	argc = argparse_parse (&argparse, argc, argv);

	//Default:
	if (findcmd == NULL) {findcmd = APP_FINDCMD;}
	if (workcmd == NULL) {workcmd = APP_WORKCMD;}
	if (xunitfilename == NULL) {xunitfilename = APP_XUNIT_FILENAME;}

	//Print selected program options:
	queue_async_add (&appresult, APP_CHANNEL_STDOUT, "\n\nargparse result:\n");
	queue_async_addf (&appresult, APP_CHANNEL_STDOUT, "options [0].flags %x\n", options [0].flags);
	queue_async_addf (&appresult, APP_CHANNEL_STDOUT, "thread_count: %d\n", thread_count);
	queue_async_addf (&appresult, APP_CHANNEL_STDOUT, "caseinfo_count: %d\n", caseinfo_count);
	queue_async_addf (&appresult, APP_CHANNEL_STDOUT, "findcmd: %s\n", findcmd);
	queue_async_addf (&appresult, APP_CHANNEL_STDOUT, "workcmd: %s\n", workcmd);
	queue_async_addf (&appresult, APP_CHANNEL_STDOUT, "logfilename: %s\n", logfilename);
	queue_async_addf (&appresult, APP_CHANNEL_STDOUT, "xunitfilename: %s\n", xunitfilename);

	//Quit when help options is enabled:
	if (options [0].flags & OPT_ENABLED)
	{
		exit (0);
	}

	//Use logfile if (logfilename) option is enabled:
	if (logfilename)
	{
		appresult.fdes [APP_CHANNEL_STDOUT] = fopen (logfilename, "w+");
		assert (appresult.fdes [APP_CHANNEL_STDOUT]);
	}

	if (xunitfilename)
	{
		appresult.fdes [APP_CHANNEL_XUNITFILE] = fopen (xunitfilename, "w+");
		assert (appresult.fdes [APP_CHANNEL_XUNITFILE]);
	}

	//Guard:
	assert (thread_count >= 0);
	assert (caseinfo_count >= 0);

	//Configure the (tsuite) which will merge test results:
	struct tsuite suite = {0};
	suite.logger = &appresult;
	suite.tcases_count = (size_t)caseinfo_count;
	suite.findcmd = findcmd;
	suite.workcmd = workcmd;
	suite.channel_xunit = APP_CHANNEL_XUNITFILE;
	suite.channel_logging = APP_CHANNEL_STDOUT;
	suite.flags = 0;
	tsuite_init (&suite);

	//Start (textlogger) thread:
	pthread_t thread_textlog;
	queue_async_init (&appresult, APP_MSG_COUNT, APP_MSG_MEM_SIZE);
	pthread_create (&thread_textlog, NULL, runner_textlog, &appresult);

	//Start all worker threads:
	pthread_t * threads = calloc ((size_t)thread_count, sizeof (pthread_t));
	for (int i = 0; i < thread_count; ++i)
	{
		queue_async_addf (&appresult, APP_CHANNEL_STDOUT, "pthread_create %i of %i\n", i, thread_count-1);
		pthread_create (threads + i, NULL, runner_suite, &suite);
	}
	//sleep (4);
	//Wait for all worker threads to complete:
	for (int i = 0; i < thread_count; ++i)
	{
		pthread_join (threads [i], NULL);
	}

	//When all worker threads are complete quit the textlogger:
	appresult.flags |= QUEUE_ASYNC_FLAG_QUIT_SOFT;
	pthread_join (thread_textlog, NULL);

	//If we have (logfilename) then we know (fdes) is a file so we can call (fclose):
	if (logfilename && appresult.fdes [APP_CHANNEL_STDOUT])
	{
		fclose (appresult.fdes [APP_CHANNEL_STDOUT]);
	}
	if (xunitfilename && appresult.fdes [APP_CHANNEL_XUNITFILE])
	{
		fclose (appresult.fdes [APP_CHANNEL_XUNITFILE]);
	}

	return EXIT_SUCCESS;
}

