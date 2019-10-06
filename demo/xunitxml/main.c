#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <errno.h>

//https://www.msweet.org/mxml/mxml.html
#include <mxml.h>

//https://www.liblfds.org/mediawiki/index.php?title=r7.1.1:Release_7.1.1_Documentation
#include <liblfds711.h>

#include <csc_readmisc.h>


#include "argparse.h"
#include "qasync.h"
#include "threads.h"
#include "tsuite.h"


#define APP_DESCRIPTION0 \
"\nThis is test-runner. It will find test and then run them in a thread pool."

#define APP_DESCRIPTION1 \
"\nNo additional description of the program is available in this version."

#define APP_XUNIT_FILENAME "default.xml"
#define APP_FINDCMD "find ../ -name \"*.c\""
#define APP_MSG_COUNT 100
#define APP_MSG_MEM_SIZE 1024
#define APP_WORKCMD "netstat"
#define APP_THREAD_COUNT 2
#define APP_CASEINFO_COUNT 11


//Channel is used for different write destinations:
//INFO channel is for logging debug information:
//XUNITFILE channel is for storing the xunit result from the test cases in a file:
enum app_channel
{
	APP_CHANNEL_INFO,
	APP_CHANNEL_INFO2
};


static const char *const usage[] =
{
"xunitxml [options] [[--] args]",
"xunitxml [options]",
NULL,
};


//Should put this in a struct somewhere?:
static pthread_cond_t condition;


//Producer thread, multiple may be used:
void * runner_suite (void * arg)
{
	struct tsuite * suite = arg;
	assert (suite);
	while (1)
	{
		if (suite->flags & TSUITE_FLAG_EMPTY) {break;}
		sleep (1);
		tsuite_runner0 (suite);
		pthread_cond_signal (&condition);
		//qasync_add(suite->logger, APP_CHANNEL_STDOUT, "Hello\n");
	}
	return NULL;
}


//Consumer thread, only one will be used:
void * runner_qasync (void * arg)
{
	pthread_mutex_t mutex;
	pthread_mutex_init (&mutex, NULL);
	struct qasync * tlog = arg;
	assert (tlog);
	while (1)
	{
		qasync_runner0 (tlog);
		if (tlog->flags & QASYNC_FLAG_EMPTY)
		{
			//Soft quit, need to wait for queue to be empty:
			if (tlog->flags & QASYNC_FLAG_QUIT) {break;}
			//Prevent busy wait by using (pthread_cond_timedwait):
			//Other threads can signal this thread that there might be data waiting in the queue:
			struct timespec ts;
			clock_gettime(CLOCK_REALTIME, &ts);
			ts.tv_sec += 1;
			pthread_mutex_lock (&mutex);
			pthread_cond_timedwait (&condition, &mutex, &ts);
			pthread_mutex_unlock (&mutex);
		}
	}
	return NULL;
}



void main_info (struct qasync * qa, char const * text)
{
	qasync_add (qa, APP_CHANNEL_INFO, text);
}


void main_infof (struct qasync * qa, char const * format, ...)
{
	va_list va;
	va_start (va, format);
	qasync_addfv (qa, APP_CHANNEL_INFO, format, va);
	va_end (va);
}



const char * whitespace_cb (mxml_node_t *node, int where)
{
	const char *element = mxmlGetElement (node);

	if
	(
	!strcmp(element, "testsuite") ||
	!strcmp(element, "testcase") ||
	!strcmp(element, "error")
	)
	{
		if (where == MXML_WS_BEFORE_OPEN ||where == MXML_WS_AFTER_CLOSE)
		{
			return ("\n");
		}
	}
	else if (!strcmp(element, "dl") || !strcmp(element, "ol") || !strcmp(element, "ul"))
	{
		return ("\n");
	}
	else if (!strcmp(element, "dd") ||!strcmp(element, "dt") ||!strcmp(element, "li"))
	{
	if (where == MXML_WS_BEFORE_OPEN) {return ("\t");}
	else if (where == MXML_WS_AFTER_CLOSE) {return ("\n");}
	}

	return (NULL);
}


/*
https://nose.readthedocs.io/en/latest/plugins/xunit.html
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
void main_generate_xmlsuite (struct tsuite * suite, char const * filename)
{
	mxmlSetWrapMargin (0);
	FILE * f = stdout;
	if (filename)
	{
		f = fopen (filename, "w+");
	}
	assert (f);
	size_t errors = 0;
	size_t failures = 0;
	size_t skip = 0;
	for (size_t i = 0; i < suite->tc_count; ++i)
	{
		if (suite->tc [i].rcode != 0)
		{
			errors ++;
		}
		if (suite->tc [i].memory != 0)
		{
			failures ++;
		}
	}
	mxml_node_t * xml = mxmlNewXML ("1.0");
	mxml_node_t * xml_suite = mxmlNewElement (xml, "testsuite");
	mxmlElementSetAttrf (xml_suite, "name", "%s", "Emulator tests");
	mxmlElementSetAttrf (xml_suite, "tests", "%zu", suite->tc_count);
	mxmlElementSetAttrf (xml_suite, "errors", "%zu", errors);
	mxmlElementSetAttrf (xml_suite, "failures", "%zu", failures);
	mxmlElementSetAttrf (xml_suite, "skip", "%zu", skip);
	for (size_t i = 0; i < suite->tc_count; ++i)
	{
		mxmlAdd (xml_suite, MXML_ADD_BEFORE, MXML_ADD_TO_PARENT, suite->tc [i].node);
		printf ("%i:%.*s\n", suite->tc [i].memory_size, suite->tc [i].memory_size, suite->tc [i].memory);
	}
	mxmlSaveFile (xml, f, whitespace_cb);
	mxmlDelete (xml);
}



int main (int argc, char const * argv [])
{
	setbuf (stdout, NULL);
	pthread_cond_init (&condition, NULL);

	//Use (resultq) to store any result from multiple threads or single thread:
	//It is designed to be thread safe:
	struct qasync resultq = {0};
	resultq.msg_count = APP_MSG_COUNT;
	resultq.fdes = calloc (2, sizeof (FILE*));
	resultq.fdes [APP_CHANNEL_INFO] = stdout; //At this stage the stdout will be used for logging information.
	resultq.fdes [APP_CHANNEL_INFO2] = NULL;

	//Init default program options:
	int thread_count = APP_THREAD_COUNT;
	int caseinfo_count = APP_CASEINFO_COUNT;
	const char *findcmd = NULL;
	const char *workcmd = NULL;
	const char *info_filename = NULL;
	const char *xunit_filename = NULL;

	//Define different program options:
	struct argparse_option options[] =
	{
		OPT_HELP(),
		OPT_GROUP("Basic options"),
		OPT_INTEGER('j', "thread_count", &thread_count, "number of threads", NULL, 0, 0),
		OPT_INTEGER('k', "caseinfo_count", &caseinfo_count, "Maximum number of testcases", NULL, 0, 0),
		OPT_STRING('f', "findcmd", &findcmd, "Use linux 'find' command here to find files to test", NULL, 0, 0),
		OPT_STRING('w', "jobcmd", &workcmd, "The command which is spread out among threads.", NULL, 0, 0),
		OPT_STRING('l', "info_filename", &info_filename, "Store info messages in info_filename", NULL, 0, 0),
		OPT_STRING('x', "xunit_filename", &xunit_filename, "Store xunit result in xunit_filename", NULL, 0, 0),
		OPT_END()
	};

	//Parse program options:
	struct argparse argparse;
	argparse_init (&argparse, options, usage, 0);
	argparse_describe (&argparse, APP_DESCRIPTION0, APP_DESCRIPTION1);
	argc = argparse_parse (&argparse, argc, argv);

	//Default:
	if (findcmd == NULL) {findcmd = APP_FINDCMD;}
	if (workcmd == NULL) {workcmd = APP_WORKCMD;}
	if (xunit_filename == NULL) {xunit_filename = APP_XUNIT_FILENAME;}

	//Print selected program options:
	main_info (&resultq, "\n\nargparse result:\n");
	main_infof (&resultq, "options [0].flags %x\n", options [0].flags);
	main_infof (&resultq, "thread_count: %d\n", thread_count);
	main_infof (&resultq, "caseinfo_count: %d\n", caseinfo_count);
	main_infof (&resultq, "findcmd: %s\n", findcmd);
	main_infof (&resultq, "workcmd: %s\n", workcmd);
	main_infof (&resultq, "logfilename: %s\n", info_filename);
	main_infof (&resultq, "xunitfilename: %s\n", xunit_filename);

	//Quit when help options is enabled:
	if (options [0].flags & OPT_ENABLED)
	{
		exit (0);
	}

	//Use logfile if (logfilename) option is enabled:
	if (info_filename)
	{
		resultq.fdes [APP_CHANNEL_INFO] = fopen (info_filename, "w+");
		assert (resultq.fdes [APP_CHANNEL_INFO]);
	}

	//Guard:
	assert (thread_count >= 0);
	assert (caseinfo_count >= 0);

	//Configure the (tsuite) which will merge test results:
	struct tsuite suite = {0};
	suite.resultq = &resultq;
	suite.tc_count = (size_t)caseinfo_count;
	suite.findcmd = findcmd;
	suite.workcmd = workcmd;
	suite.channel_xunit = APP_CHANNEL_INFO2;
	suite.channel_info = APP_CHANNEL_INFO;
	suite.flags = 0;
	tsuite_init (&suite);

	//Start (qasync) thread:
	pthread_t thread_textlog;
	qasync_init (&resultq, APP_MSG_MEM_SIZE);
	pthread_create (&thread_textlog, NULL, runner_qasync, &resultq);

	//Start all worker threads:
	pthread_t * threads = calloc ((size_t)thread_count, sizeof (pthread_t));
	for (int i = 0; i < thread_count; ++i)
	{
		main_infof (&resultq, "pthread_create %i of %i\n", i, thread_count-1);
		pthread_create (threads + i, NULL, runner_suite, &suite);
	}
	//sleep (4);
	//Wait for all worker threads to complete:
	for (int i = 0; i < thread_count; ++i)
	{
		pthread_join (threads [i], NULL);
	}

	//When all worker threads are complete quit the textlogger:
	resultq.flags |= QASYNC_FLAG_QUIT;
	pthread_join (thread_textlog, NULL);

	//If we have (logfilename) then we know (fdes) is a file so we can call (fclose):
	if (info_filename && resultq.fdes [APP_CHANNEL_INFO])
	{
		fclose (resultq.fdes [APP_CHANNEL_INFO]);
	}

	main_generate_xmlsuite (&suite, xunit_filename);

	tsuite_cleanup (&suite);
	qasync_cleanup (&resultq);

	return EXIT_SUCCESS;
}







