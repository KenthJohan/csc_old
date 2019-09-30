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


#define TCASE_COUNT 11
#define TCASE_START_MEMORY_SIZE 128
#define TCASE_FILENAME_MAXLEN 128

#define THREAD_COUNT 2
#define THREAD_PRIORITY 1
#define THREAD_POLICY SCHED_RR
//#define THREAD_POLICY SCHED_FIFO
//#define THREAD_POLICY SCHED_OTHER



void threads_create (pthread_t t [], unsigned n, void *(* func)(void *), struct lfds711_stack_state * ss, int policy, int priority)
{
	pthread_attr_t a;
	pthread_attr_init(&a);
	pthread_attr_setschedpolicy (&a, policy);
	struct sched_param p;
	pthread_attr_getschedparam (&a, &p);
	p.sched_priority = priority;
	pthread_attr_setschedparam (&a, &p);
	while (n--)
	{
		pthread_create (t + n, &a, func, ss);
	}
}


void threads_join (pthread_t t [], size_t n)
{
	while (n--)
	{
		pthread_join (t[n], NULL);
	}
}

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
	int r;
	FILE * fp = popen ("netstat", "r");
	item->memory_size = TCASE_START_MEMORY_SIZE;
	item->memory = csc_readmisc_realloc (fileno (fp), &item->memory_size);
	r = pclose (fp);
	assert (item->memory);
}


void * caseinfo_runner (void * arg)
{
	assert (arg);
	struct lfds711_stack_state * ss = arg;
	while (1)
	{
		struct lfds711_stack_element * se;
		struct caseinfo * td;
		int r = lfds711_stack_pop (ss, &se);
		sleep (rand() % 3);
		if (r == 0) {break;}
		td = LFDS711_STACK_GET_VALUE_FROM_ELEMENT (*se);
		caseinfo_popen (td);
		printf ("===========\nid %i. n %i. pclose %i\n%.*s\n", td->id, td->memory_size, r, 64, td->memory);
	}
	return NULL;
}


void caseinfo_populate_filename (struct caseinfo items [], size_t n, char const * cmd)
{
	assert (items);
	printf ("cmd %s\n", cmd);
	FILE * fp = popen (cmd, "r");
	assert (fp);
	while (n--)
	{
		items [n].filename = malloc (TCASE_FILENAME_MAXLEN);
		assert (items [n].filename);
		char * r = fgets (items [n].filename, TCASE_FILENAME_MAXLEN, fp);
		if (r == NULL) {break;}
		fputs (items [n].filename, stdout);
	}
	int r = pclose (fp);
	printf ("pclose %i\n", r);
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


int main (int argc, char * argv [])
{
	struct lfds711_stack_state ss;
	lfds711_stack_init_valid_on_current_logical_core (&ss, NULL);
	struct caseinfo * tcases = malloc (sizeof(struct caseinfo) * TCASE_COUNT);
	caseinfo_populate_filename (tcases, TCASE_COUNT, "find ../ -name \"*.h\"");
	caseinfo_populate_ss (tcases, TCASE_COUNT, &ss);
	pthread_t threads [THREAD_COUNT];
	threads_create (threads, THREAD_COUNT, caseinfo_runner, &ss, THREAD_POLICY, THREAD_PRIORITY);
	threads_join (threads, THREAD_COUNT);
	lfds711_stack_cleanup (&ss, NULL);
	free (tcases);
	return EXIT_SUCCESS;
}

