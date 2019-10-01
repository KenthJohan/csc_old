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


#define CASEINFO_COUNT 11
#define CASEINFO_START_MEMORY_SIZE 128
#define CASEINFO_FILENAME_MAXLEN 128
#define CASEINFO_FINDCMD "find ../ -name \"*.c\""

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

struct caseinfo
{
	struct lfds711_stack_element se;
	int id;
	unsigned memory_size;
	char * memory;
	char * filename;
};


void caseinfo_popen (struct caseinfo * item)
{
	FILE * fp = popen ("netstat", "r");
	item->memory_size = CASEINFO_START_MEMORY_SIZE;
	item->memory = csc_readmisc_realloc (fileno (fp), &item->memory_size);
	int r = pclose (fp);
	textlog_addf (&textlogger, "caseinfo_popen : pclose %i\n", r);
	assert (item->memory);
}


void * caseinfo_runner (void * arg)
{
	assert (arg);
	struct lfds711_stack_state * ss = arg;
	while (1)
	{
		struct lfds711_stack_element * se;
		struct caseinfo * cinfo;
		int r = lfds711_stack_pop (ss, &se);
		if (r == 0) {break;}
		cinfo = LFDS711_STACK_GET_VALUE_FROM_ELEMENT (*se);
		textlog_addf (&textlogger, "lfds711_stack_pop %i\n", cinfo->id);
		sleep (rand() % 2); //Sleep just for simulation
		caseinfo_popen (cinfo);
		textlog_addf (&textlogger, "===========\nid %i. n %i. pclose %i\n%.*s\n", cinfo->id, cinfo->memory_size, r, 64, cinfo->memory);
	}
	return NULL;
}


void caseinfo_populate_filename (struct caseinfo items [], size_t n, char const * cmd)
{
	if (n == 0) {return;}
	assert (items);
	textlog_addf (&textlogger, "cmd %s\n", cmd);
	FILE * fp = popen (cmd, "r");
	assert (fp);
	while (n--)
	{
		items [n].filename = malloc (CASEINFO_FILENAME_MAXLEN);
		assert (items [n].filename);
		char * r = fgets (items [n].filename, CASEINFO_FILENAME_MAXLEN, fp);
		if (r == NULL) {break;}
		fputs (items [n].filename, stdout);
	}
	int r = pclose (fp);
	textlog_addf (&textlogger, "pclose %i\n", r);
}


void caseinfo_populate_ss (struct caseinfo items [], size_t n, struct lfds711_stack_state * ss)
{
	assert (items);
	assert (ss);
	while (n--)
	{
		items [n].id = (int)n;
		LFDS711_STACK_SET_VALUE_IN_ELEMENT (items[n].se, &items[n]);
		lfds711_stack_push (ss, &items[n].se);
	}
}

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
	struct argparse_option options[] =
	{
		OPT_HELP(),
		OPT_GROUP("Basic options"),
		OPT_INTEGER('j', "thread_count", &thread_count, "number of threads", NULL, 0, 0),
		OPT_INTEGER('s', "thread_policy", &thread_policy, "policy of each thread", NULL, 0, 0),
		OPT_INTEGER('n', "thread_priority", &thread_priority, "priority of each thread", NULL, 0, 0),
		OPT_INTEGER('k', "caseinfo_count", &caseinfo_count, "Maximum number of testcases", NULL, 0, 0),
		OPT_STRING('p', "findcmd", &findcmd, "Use linux 'find' command here to find files to test", NULL, 0, 0),
		OPT_END()
	};
	struct argparse argparse;
	argparse_init(&argparse, options, usage, 0);
	argparse_describe(&argparse, DESCRIPTION0, DESCRIPTION1);
	argc = argparse_parse(&argparse, argc, argv);

	printf ("\n\nargparse result:\n");
	printf ("thread_count: %d\n", thread_count);
	printf ("thread_policy: %d\n", thread_policy);
	printf ("thread_priority: %d\n", thread_priority);
	printf ("caseinfo_count: %d\n", caseinfo_count);
	if (findcmd == NULL) {findcmd = CASEINFO_FINDCMD;}
	printf ("findcmd: %s\n", findcmd);
	printf ("options [0].flags %x\n", options [0].flags);
	if (options [0].flags & OPT_ENABLED)
	{
		exit (0);
	}

	assert (thread_count >= 0);
	assert (caseinfo_count >= 0);
	assert (thread_policy == SCHED_RR || thread_policy == SCHED_FIFO || thread_policy == SCHED_OTHER);



	textlog_start (&textlogger, 20);
	struct lfds711_stack_state ss;
	lfds711_stack_init_valid_on_current_logical_core (&ss, NULL);
	struct caseinfo * tcases = malloc (sizeof(struct caseinfo) * (size_t)caseinfo_count);
	caseinfo_populate_filename (tcases, (size_t)caseinfo_count, findcmd);
	caseinfo_populate_ss (tcases, (size_t)caseinfo_count, &ss);
	pthread_t * threads = malloc (sizeof (pthread_t) * (size_t)thread_count);
	threads_create (threads, (size_t)thread_count, caseinfo_runner, &ss, thread_policy, thread_priority);
	threads_join (threads, (size_t)thread_count);
	printf ("TEXTLOG_FLAG_QUIT\n");
	textlogger.flag = TEXTLOG_FLAG_QUIT;
	pthread_join (textlogger.thread, NULL);
	lfds711_stack_cleanup (&ss, NULL);
	free (tcases);
	return EXIT_SUCCESS;
}

