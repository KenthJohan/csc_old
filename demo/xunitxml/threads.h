#pragma once
#include <pthread.h>



static void thread_set (pthread_attr_t * a, int policy, int priority)
{
	pthread_attr_init(a);
	pthread_attr_setschedpolicy (a, policy);
	struct sched_param p;
	pthread_attr_getschedparam (a, &p);
	p.sched_priority = priority;
	pthread_attr_setschedparam (a, &p);
}


static void threads_create (pthread_t t [], size_t n, void *(* func)(void *), void * arg, int policy, int priority)
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
		pthread_create (t + n, &a, func, arg);
	}
}


static void threads_join (pthread_t t [], size_t n)
{
	while (n--)
	{
		pthread_join (t[n], NULL);
	}
}
