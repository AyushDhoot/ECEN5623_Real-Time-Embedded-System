/******************************************************************************************
*	@file		threadindexed.c
*	@brief		Thread safety and reenterancy exibition with thread local storage.
				Pertains to problem 2 of Exercise 3 RTES.
*	@Tools_Used	GCC  
*	@Author		Souvik De
*	@Date		03/02/2019
*
*******************************************************************************************/

#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<unistd.h>
#include<sys/syscall.h>

typedef struct my_thread
{
	int a;
	int b;

}thread_t;

__thread int c;

void swap(int *a, int *b)
{
	c = *a;
	*a = *b;
	*b = c;
}

void task(void *requesting_thread)
{
	thread_t * req_thread = (thread_t*) requesting_thread;
	int a,b;

	a = req_thread->a;
	b = req_thread->b;

	printf("Thread %ld - Before swap: a = %d, b = %d\n",syscall(__NR_gettid),a,b);
	swap(&a,&b);
	printf("Thread %ld - After swap: a = %d, b = %d\n",syscall(__NR_gettid),a,b);
}

int main()
{
	pthread_t thread1, thread2;	
	int thread_status = 0;
	thread_t *t_child1 = NULL;
	thread_t *t_child2 = NULL;

	t_child1 = (thread_t*)malloc(sizeof(thread_t));
	if(t_child1 == NULL)
	{
		printf("\nMalloc for Child Thread-1 failed. Exiting\n");
		return 0;
	}
	t_child1->a = 2;
	t_child1->b = 8;

	t_child2 = (thread_t*)malloc(sizeof(thread_t));
	if(t_child2 == NULL)
	{
		printf("\nMalloc for Child Thread-2 failed. Exiting\n");
		free(t_child1);
		return 0;
	}
	t_child2->a = 5;
	t_child2->b = 9;

	thread_status = pthread_create(&thread1,NULL,(void *)task, (void*) t_child1);
	if(thread_status != 0)
	{
		printf("\nChild thread - 1 cannot be created\n");
		return 0;
	}

	thread_status = pthread_create(&thread2,NULL,(void *)task, (void*) t_child2);
	if(thread_status != 0)
	{
		printf("\nChild thread - 2 cannot be created\n");
		return 0;
	}

	pthread_join(thread1, NULL);
	pthread_join(thread2, NULL);

	return 0;
}