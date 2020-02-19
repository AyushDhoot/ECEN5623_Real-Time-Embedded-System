/******************************************************************************************
*	@file		accelero.c
*	@brief		This problem pertains to the last part of problem 2 in 
				Exercise 3 of RTES. It involves calculaing accelerome-
				ter parameters using one thread and reading it with a-
				nother, without corrupting the previous data.
*	@Tools_Used	GCC  
*	@Author		Souvik De
*	@Date		03/03/2019
*
*******************************************************************************************/

#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<time.h>
#include<math.h>
#include<unistd.h>
#include<sys/time.h>
#include<sys/syscall.h>

#define PI 	(3.14)  

pthread_mutex_t resource_lock;

typedef struct
{
	int id;
	struct timespec	timestamp;
	double acc_x;
	double acc_y;
	double acc_z;
	double roll;
	double pitch;
	double yaw;
}obj_t;

obj_t complex_obj;

void calculate_data(void *requesting_thread)
{
	obj_t *req_thread = (obj_t*) requesting_thread;
	pthread_mutex_lock(&resource_lock);

	complex_obj.acc_x = req_thread->acc_x;
	complex_obj.acc_y = req_thread->acc_y;
	complex_obj.acc_z = req_thread->acc_z;
	complex_obj.pitch = 180 * atan(complex_obj.acc_x/sqrt(complex_obj.acc_y*complex_obj.acc_y + complex_obj.acc_z*complex_obj.acc_z))/PI;
	complex_obj.roll = 180 * atan(complex_obj.acc_y/sqrt(complex_obj.acc_x*complex_obj.acc_x + complex_obj.acc_z*complex_obj.acc_z))/PI;
	complex_obj.yaw = 180 * atan (complex_obj.acc_z/sqrt(complex_obj.acc_x*complex_obj.acc_x + complex_obj.acc_z*complex_obj.acc_z))/PI;
	clock_gettime(CLOCK_REALTIME, &complex_obj.timestamp);

	pthread_mutex_unlock(&resource_lock);
}

void read_data(void *requesting_thread)
{
	obj_t *req_thread = (obj_t*) requesting_thread;
	pthread_mutex_lock(&resource_lock);

	req_thread->acc_x = complex_obj.acc_x;
	req_thread->acc_y = complex_obj.acc_y;
	req_thread->acc_z = complex_obj.acc_z;
	req_thread->roll = complex_obj.roll;
	req_thread->pitch = complex_obj.pitch;
	req_thread->yaw = complex_obj.yaw;
	req_thread->timestamp.tv_sec = complex_obj.timestamp.tv_sec;
	req_thread->timestamp.tv_nsec = complex_obj.timestamp.tv_nsec;

	printf("Acc_x = %lf\n",req_thread->acc_x);
	printf("Acc_y = %lf\n",req_thread->acc_y);
	printf("Acc_z = %lf\n",req_thread->acc_z);
	printf("Roll = %lf\n",req_thread->roll);
	printf("Pitch = %lf\n",req_thread->pitch);
	printf("Yaw = %lf\n",req_thread->yaw);
	printf("Timestamp: Sec = %lu, Nano Sec = %lu\n",req_thread->timestamp.tv_sec,req_thread->timestamp.tv_nsec);

	pthread_mutex_unlock(&resource_lock);
}

void task(void *requesting_thread)
{
	obj_t *req_thread = (obj_t*) requesting_thread;

	while(1)
	{
		if(req_thread->id == 1)
		{
			srand(time(0));
			req_thread->acc_x = (double)((rand() % 256) + 12.567); 
			req_thread->acc_y = (double)((rand() % 125) + 5.46); 
			req_thread->acc_z = (double)((rand() % 50) + 45.67); 
			calculate_data(req_thread);
			printf("Thread %ld - Finished Calculating Data\n",syscall(__NR_gettid));
			sleep(1);
		}
		else
		{
			read_data(req_thread);
			printf("Thread %ld - Finished Reading Data\n",syscall(__NR_gettid));
			sleep(1);
		}
	}

}

int main()
{
	pthread_t thread1, thread2;	
	int thread_status = 0;
	obj_t *obj1 = NULL;
	obj_t *obj2 = NULL;

	pthread_attr_t child1_attr, child2_attr;
	struct sched_param child1_param, child2_param, main_param;

	sched_getparam(getpid(), &main_param);
	int max_prio = sched_get_priority_max(SCHED_FIFO);
	main_param.sched_priority = max_prio;
   	sched_setscheduler(getpid(), SCHED_FIFO, &main_param);

	pthread_attr_init(&child1_attr);
  	pthread_attr_init(&child2_attr);

  	pthread_attr_setschedpolicy(&child1_attr, SCHED_FIFO);
  	pthread_attr_setschedpolicy(&child2_attr, SCHED_FIFO);

  	child1_param.sched_priority = max_prio-1;
  	child2_param.sched_priority = max_prio-2;

  	pthread_attr_setschedparam(&child1_attr, &child1_param);
  	pthread_attr_setschedparam(&child2_attr, &child2_param);


	obj1 = (obj_t*)malloc(sizeof(obj_t));
	if(obj1 == NULL)
	{
		printf("\nMalloc for Object-1 failed. Exiting\n");
		return 0;
	}
	obj1->id = 1;

	obj2 = (obj_t*)malloc(sizeof(obj_t));
	if(obj2 == NULL)
	{
		printf("\nMalloc for Object-2 failed. Exiting\n");
		return 0;
	}
	obj2->id = 2;

	if(pthread_mutex_init(&resource_lock, NULL) != 0)
	{
		printf("\nMutex initialization failed. Exiting\n");
		return 0;
	}

	thread_status = pthread_create(&thread1,&child1_attr,(void *)task, (void*) obj1);
	if(thread_status != 0)
	{
		printf("\nChild thread - 1 cannot be created\n");
		return 0;
	}

	thread_status = pthread_create(&thread2,&child2_attr,(void *)task, (void*) obj2);
	if(thread_status != 0)
	{
		printf("\nChild thread - 2 cannot be created\n");
		return 0;
	}

	pthread_join(thread1, NULL);
	pthread_join(thread2, NULL);

	pthread_mutex_destroy(&resource_lock);

	return 0;
}
