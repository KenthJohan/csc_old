#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>

#include <mxml.h>
#include <liblfds711.h>


static void * worker (void *arg)
{
	/* Wait until tasks is available. */
	//while (!queue_get(pool->pend_q, &t_ptr))
	{
		/* And then execute it. */
	}
}



struct test_data
{
	struct lfds711_stack_element se;
	int long long unsigned user_id;
};

int main (int argc, char * argv [])
{
	int long long unsigned loop;
	struct lfds711_stack_element *se;
	struct lfds711_stack_state ss;
	struct test_data *td, *temp_td;
	lfds711_stack_init_valid_on_current_logical_core( &ss, NULL );
	td = malloc( sizeof(struct test_data) * 10 );
	for( loop = 0 ; loop < 10 ; loop++ )
	{
		td[loop].user_id = loop;
		LFDS711_STACK_SET_VALUE_IN_ELEMENT (td[loop].se, &td[loop]);
		lfds711_stack_push (&ss, &td[loop].se);
	}

	for( loop = 0 ; loop < 10 ; loop++ )
	{
		lfds711_stack_pop( &ss, &se );
		temp_td = LFDS711_STACK_GET_VALUE_FROM_ELEMENT (*se);
		printf ("user_id = %llu\n", temp_td->user_id);
	}

	lfds711_stack_cleanup (&ss, NULL);
	free (td);
	return EXIT_SUCCESS;
}

