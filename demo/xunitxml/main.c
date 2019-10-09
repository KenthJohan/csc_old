#define __USE_MINGW_ANSI_STDIO 1
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
#include <inttypes.h>

//https://www.msweet.org/mxml/mxml.html
#include <mxml.h>

//https://www.liblfds.org/mediawiki/index.php?title=r7.1.1:Release_7.1.1_Documentation
#include <liblfds711.h>


#include "argparse.h"
#include "qasync.h"
#include "tsuite.h"


#define APP_DESCRIPTION0 \
"\nTest runner for emulator test. It will run selected test and compose a xunit xml report."

#define APP_DESCRIPTION1 \
"\nTip: Use \"time xunitxml -j<threads>\" to find out how many threads gives the best execution time."

#define APP_XUNIT_FILENAME "default.xml"
#define APP_FINDCMD "find . -name \"*.emuTest\""
#define APP_MSG_COUNT 100
#define APP_MSG_MEM_SIZE 1024
#define APP_WORKCMD "build\\spacex7_emu.exe -f %s 2>&1"
#define APP_THREAD_COUNT 2
#define APP_CASEINFO_COUNT 11
#define APP_ASSERTGREP "Assert FAIL"


//Channel is used for different write destinations:
//INFO channel is for logging debug information:
//XUNITFILE channel is for storing the xunit result from the test cases in a file:
enum app_channel
{
	APP_CHANNEL_INFO,
	APP_CHANNEL_INFO2,
	APP_CHANNEL_COUNT
};


static const char *const usage[] =
{
"xunitxml [options] [[--] args]",
"xunitxml [options]",
NULL,
};


//Should put this in a struct somewhere?:
static pthread_cond_t condition;


//Producer thread function.
//Many threads may be used.
void * runner_suite (void * arg)
{
	struct tsuite * suite = arg;
	assert (suite);
	while (1)
	{
		if (suite->flags & TSUITE_FLAG_EMPTY) {break;}
		tsuite_runner0 (suite);
		pthread_cond_signal (&condition);
		//qasync_add(suite->logger, APP_CHANNEL_STDOUT, "Hello\n");
	}
	return NULL;
}


//Consumer thread function.
//Only one thread should be used.
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
			//Prevent busy-wait by using (pthread_cond_timedwait).
			//Other threads can wakeup this thread by calling (pthread_cond_signal).
			//Using timeout might be temporary for now.
			struct timespec ts;
			clock_gettime (CLOCK_REALTIME, &ts);
			//ts.tv_sec += 1;
			ts.tv_nsec += 100000;
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
		if (where == MXML_WS_BEFORE_OPEN || where == MXML_WS_AFTER_CLOSE)
		{
			return ("\n");
		}
	}
	return NULL;
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
	assert (suite);
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
		if (suite->tc [i].exit_status != 0)
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
		//printf ("%i:%.*s\n", suite->tc [i].memory_size, suite->tc [i].memory_size, suite->tc [i].memory);
	}
	mxmlSaveFile (xml, f, whitespace_cb);
	mxmlDelete (xml);
}


int main (int argc, char const * argv [])
{
	struct tsuite suite = {0};
	struct qasync resq = {0};

	setbuf (stdout, NULL);
	mxmlSetWrapMargin (0);
	pthread_cond_init (&condition, NULL);

	//Use (resultq) to store any result from multiple threads or single thread:
	//It is designed to be thread safe:
	resq.msg_count = APP_MSG_COUNT;
	resq.fdes = calloc (APP_CHANNEL_COUNT, sizeof (FILE*));
	resq.fdes [APP_CHANNEL_INFO] = stdout; //At this stage the stdout will be used for logging information.
	resq.fdes [APP_CHANNEL_INFO2] = NULL;

	//Init default program options:
	int thread_count = APP_THREAD_COUNT;
	int caseinfo_count = APP_CASEINFO_COUNT;
	const char *loginfo_filename = NULL;
	const char *xunitxml_filename = NULL;

	//Define different program options:
	struct argparse_option options[] =
	{
		OPT_HELP(),
		OPT_GROUP("Basic options"),
		OPT_INTEGER('j', "thread_count", &thread_count, "Number of threads. 0 means everything executes sequently.", NULL, 0, 0),
		OPT_INTEGER('k', "caseinfo_count", &caseinfo_count, "Maximum number of testcases.", NULL, 0, 0),
		OPT_STRING('f', "findcmd", &suite.findcmd, "This command selects testfiles. e.g. (find . -name \"*.emuTest\")", NULL, 0, 0),
		OPT_STRING('w', "workcmd", &suite.workcmd, "This command is invoked per testfile. Use (%s) to insert each filename in respective command.", NULL, 0, 0),
		OPT_STRING('l', "loginfo_filename", &loginfo_filename, "If defined then program will create this file and put all loginfo there.", NULL, 0, 0),
		OPT_STRING('x', "xunitxml_filename", &xunitxml_filename, "Save the xunit xml report in this file.", NULL, 0, 0),
		OPT_STRING('a', "assertgrep", &suite.assertgrep, "This search-string is used to find the assert message line from (workcmd).", NULL, 0, 0),
		OPT_END()
	};

	//Parse program options:
	struct argparse argparse;
	argparse_init (&argparse, options, usage, 0);
	argparse_describe (&argparse, APP_DESCRIPTION0, APP_DESCRIPTION1);
	argc = argparse_parse (&argparse, argc, argv);

	//Default:
	if (suite.findcmd == NULL) {suite.findcmd = APP_FINDCMD;}
	if (suite.workcmd == NULL) {suite.workcmd = APP_WORKCMD;}
	if (xunitxml_filename == NULL) {xunitxml_filename = APP_XUNIT_FILENAME;}
	if (suite.assertgrep == NULL) {suite.assertgrep = APP_ASSERTGREP;}

	//Print selected program options:
	main_info (&resq, "\n\nargparse result:\n");
	main_infof (&resq, "%30.30s : %x\n", "options [0].flags", options [0].flags);
	main_infof (&resq, "%30.30s : %d\n", "thread_count", thread_count);
	main_infof (&resq, "%30.30s : %d\n", "caseinfo_count", caseinfo_count);
	main_infof (&resq, "%30.30s : %s\n", "findcmd", suite.findcmd);
	main_infof (&resq, "%30.30s : %s\n", "workcmd", suite.workcmd);
	main_infof (&resq, "%30.30s : %s\n", "loginfo_filename", loginfo_filename);
	main_infof (&resq, "%30.30s : %s\n", "xunitxml_filename", xunitxml_filename);
	main_infof (&resq, "%30.30s : %s\n", "assertgrep", suite.assertgrep);

	//Quit when help options is enabled:
	if (options [0].flags & OPT_ENABLED)
	{
		exit (0);
	}

	//Use logfile if (logfilename) option is enabled:
	if (loginfo_filename)
	{
		resq.fdes [APP_CHANNEL_INFO] = fopen (loginfo_filename, "w+");
		assert (resq.fdes [APP_CHANNEL_INFO]);
	}

	//Guard:
	assert (thread_count >= 0);
	assert (caseinfo_count >= 0);

	//Configure the (tsuite) which will merge test results:
	suite.resultq = &resq;
	suite.tc_count = (size_t)caseinfo_count;
	suite.channel_xunit = APP_CHANNEL_INFO2;
	suite.channel_info = APP_CHANNEL_INFO;
	suite.flags = 0;
	tsuite_init (&suite);

	//Start all worker threads:
	if (thread_count > 0)
	{
		//Start (qasync) thread:
		pthread_t thread_textlog;
		qasync_init (&resq, APP_MSG_MEM_SIZE);
		pthread_create (&thread_textlog, NULL, runner_qasync, &resq);

		pthread_t * threads = calloc ((size_t)thread_count, sizeof (pthread_t));
		assert (threads);
		for (int i = 0; i < thread_count; ++i)
		{
			pthread_create (threads + i, NULL, runner_suite, &suite);
			main_infof (&resq, "thread=%04i created\n", (unsigned)threads [i]);
		}

		//Wait until each worker thread is complete:
		for (int i = 0; i < thread_count; ++i)
		{
			pthread_join (threads [i], NULL);
			main_infof (&resq, "thread=%04i complete\n", (unsigned)threads [i]);
		}

		//Convert the (resq) to non threaded mode:
		resq.flags |= QASYNC_FLAG_QUIT;
		main_infof (&resq, "Waiting for loginfo thread to complete\n");
		pthread_join (thread_textlog, NULL);
		resq.flags &= ~QASYNC_FLAG_THREADMODE;
	}
	else
	{
		for (size_t i = 0; i < suite.tc_count; ++i)
		{
			tsuite_runner1 (&suite, suite.tc + i);
		}
		main_infof (&resq, "runner_suite %s end\n", "");
	}

	//If we have (logfilename) then we know (fdes) is a file so we can call (fclose):
	if (loginfo_filename && resq.fdes [APP_CHANNEL_INFO])
	{
		fclose (resq.fdes [APP_CHANNEL_INFO]);
	}


	main_infof (&resq, "main_generate_xmlsuite %s\n", xunitxml_filename);
	main_generate_xmlsuite (&suite, xunitxml_filename);

	tsuite_cleanup (&suite);
	qasync_cleanup (&resq);

	return EXIT_SUCCESS;
}

