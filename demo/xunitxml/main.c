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

#include <csc_blksread.h>


#define TEST_COUNT 14
#define THREADS 3
//#define POLICY SCHED_FIFO
//#define POLICY SCHED_OTHER
#define POLICY SCHED_RR
#define PRIORITY 1
#define BLOCK_COUNT 4
#define BLOCK_SIZE 64


struct test_data
{
	struct lfds711_stack_element se;
	int long long unsigned user_id;
	int last;
	char * block [BLOCK_COUNT];
};




static void * worker (void *arg)
{
	assert (arg);
	struct lfds711_stack_state * ss = arg;
	while (1)
	{
		struct lfds711_stack_element * se;
		struct test_data * td;
		int r = lfds711_stack_pop (ss, &se);
		sleep (rand() % 7);
		if (r == 0) {break;}
		td = LFDS711_STACK_GET_VALUE_FROM_ELEMENT (*se);
		printf ("user_id = %llu\n", td->user_id);
	}
	return NULL;
}


void create_threads (pthread_t t [], unsigned n, struct lfds711_stack_state * ss, int policy, int priority)
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
		pthread_create (t + n, &a, worker, ss);
	}
}


void open_process ()
{
	int r;
	char * block [BLOCK_COUNT] = {NULL};
	FILE * fp = popen ("netstat", "r");
	r = csc_blksread (fileno (fp), block, BLOCK_COUNT, BLOCK_SIZE);
	printf ("reader %i\n", r);
	if (r < 0)
	{
		perror ("reader failed");
	}

	printf ("Block %i:\n%.*s\n", 0, BLOCK_SIZE, block [0]);
	printf ("Block %i:\n%.*s\n", 1, BLOCK_SIZE, block [1]);
	printf ("Block %i:\n%.*s\n", 2, BLOCK_SIZE, block [2]);
	printf ("Block %i:\n%.*s\n", 3, BLOCK_SIZE, block [3]);
	r = pclose (fp);
	printf ("r %i\n", r);
}


int main (int argc, char * argv [])
{
	/*
	struct lfds711_stack_state ss;
	struct test_data *td;
	lfds711_stack_init_valid_on_current_logical_core (&ss, NULL);
	td = malloc( sizeof(struct test_data) * TEST_COUNT );
	for (int long long unsigned i = 0; i < TEST_COUNT; ++i)
	{
		td[i].user_id = i;
		LFDS711_STACK_SET_VALUE_IN_ELEMENT (td[i].se, &td[i]);
		lfds711_stack_push (&ss, &td[i].se);
	}

	pthread_t threads[THREADS];
	create_threads (threads, THREADS, &ss, POLICY, PRIORITY);
	for (int i = 0; i < THREADS; i++)
	{
		pthread_join(threads[i], NULL);
	}

	lfds711_stack_cleanup (&ss, NULL);
	free (td);
	*/
	open_process ();


	return EXIT_SUCCESS;
}

