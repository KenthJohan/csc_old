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


#define TEST_COUNT 100
#define THREADS 3




struct test_data
{
	struct lfds711_stack_element se;
	int long long unsigned user_id;
};

static void * worker (void *arg)
{
	struct lfds711_stack_state * ss = arg;
	while (1)
	{
		struct lfds711_stack_element * se;
		struct test_data * td;
		int r = lfds711_stack_pop (ss, &se);
		sleep (rand() % 5);
		if (r == 0) {break;}
		td = LFDS711_STACK_GET_VALUE_FROM_ELEMENT (*se);
		printf ("user_id = %llu\n", td->user_id);
	}
}

int main (int argc, char * argv [])
{
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
	for (int i = 0; i < THREADS; i++)
	{
		pthread_create (&threads[i], NULL, worker, &ss);
	}
	for (int i = 0; i < THREADS; i++)
	{
		pthread_join(threads[i], NULL);
	}

	//for( loop = 0 ; loop < 10 ; loop++ )
	{
		//lfds711_stack_pop (&ss, &se);
		//temp_td = LFDS711_STACK_GET_VALUE_FROM_ELEMENT (*se);
		//printf ("user_id = %llu\n", temp_td->user_id);
	}

	lfds711_stack_cleanup (&ss, NULL);
	free (td);
	return EXIT_SUCCESS;
}

