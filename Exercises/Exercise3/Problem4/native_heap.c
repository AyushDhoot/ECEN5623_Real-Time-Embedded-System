/******************************************************************************************
*	@file		native_heap.c
*	@brief		Pthread FIFO adaptation of VxWorks heap_mq.c
*	@Tools_Used	GCC  
*	@Author		Souvik De
*	@Date		03/04/2019
*
*******************************************************************************************/

#include<fcntl.h> 
#include<sys/stat.h>
#include<mqueue.h>
#include<stdio.h>
#include<stdlib.h>
#include<stdint.h>
#include<string.h>
#include<unistd.h>
#include<errno.h>
#include<sys/time.h>
#include<pthread.h>
#include<unistd.h>
#include<sys/syscall.h>
#include<signal.h>

#define Q_NAME 		("/my_queue1")
#define Q_SIZE		(8)

mqd_t msgqueue_FD;
struct mq_attr msgqueue_FD_attr;

static char imagebuff[4096];
static int sid, rid;

void sender(void)
{
	int value;

	char buffer[sizeof(void *)+sizeof(int)];
  	void *buffptr;
  	int id = 999;

	msgqueue_FD = mq_open(Q_NAME, O_CREAT | O_RDWR, 0666, &msgqueue_FD_attr);	

	if(msgqueue_FD == (mqd_t)-1)
	{
		perror("Error in opening Message Queue");
		exit(1);
	}

	while(1)
	{
		buffptr = (void *)malloc(sizeof(imagebuff));
    	strcpy(buffptr, imagebuff);
    	printf("Message to send = %s\n", (char *)buffptr);

    	printf("Sending %ld bytes\n", sizeof(buffptr));

    	memcpy(buffer, &buffptr, sizeof(void *));
    	memcpy(&(buffer[sizeof(void *)]), (void *)&id, sizeof(int));

		value = mq_send(msgqueue_FD, (char*)buffer, (size_t)(sizeof(void *)+sizeof(int)),30);	
		if(value == -1)
		{
			perror("Error in sending message");
		}
		else
    	{
      		printf("Send: message ptr 0x%p successfully sent\n", buffptr);
    	}

    	sleep(3);
	}
}

void receiver(void)
{
	int value;

	char buffer[sizeof(void *)+sizeof(int)];
  	void *buffptr; 
  	int count = 0;
  	int id;
  	int prio;

	msgqueue_FD = mq_open(Q_NAME, O_CREAT | O_RDWR, 0666, &msgqueue_FD_attr);
	if(msgqueue_FD == -1)
	{
		perror("Error in opennig message queue");
		exit(1);
	}

	while(1)
	{
		printf("Reading %ld bytes\n", sizeof(void *));

		value = mq_receive(msgqueue_FD, buffer, (size_t)(sizeof(void *)+sizeof(int)),&prio);
		if(value == -1)
		{
			perror("Error in receiving message");
		}
		else
		{
			memcpy(&buffptr, buffer, sizeof(void *));
      		memcpy((void *)&id, &(buffer[sizeof(void *)]), sizeof(int));
      		printf("Receive: ptr msg %p, received with priority = %d, length = %d, id = %d\n", buffptr,prio,value,id);

      		printf("Contents of ptr = %s\n", (char *)buffptr);

      		free(buffptr);

      		printf("Heap space memory freed\n");
		}

	}
}

void _handler_kill(int signal)
{
	printf("Killed by Ctrl-C\n");
	mq_close(msgqueue_FD);
	mq_unlink(Q_NAME);
	exit(1);
}

int main()
{
	pthread_t thread1, thread2;	
	int thread_status = 0;

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

	struct sigaction kill_action;
	memset (&kill_action, 0, sizeof (kill_action));
	kill_action.sa_handler = &_handler_kill;
	sigaction (SIGINT, &kill_action, NULL);	

	msgqueue_FD_attr.mq_maxmsg = Q_SIZE;
	msgqueue_FD_attr.mq_msgsize = sizeof(void *)+sizeof(int);

	int i, j;
  	char pixel = 'A';

  	for(i = 0 ; i < 4096 ; i += 64)
  	{
    	pixel = 'A';
    	for(j = i ; j < i + 64 ; j++) 
    	{
      		imagebuff[j] = (char)pixel++;
    	}
    	imagebuff[j-1] = '\n';
  	}
  	imagebuff[4095] = '\0';
  	imagebuff[63] = '\0';

  	printf("buffer =%s\n", imagebuff);

	thread_status = pthread_create(&thread1,&child1_attr,(void *)receiver, NULL);
	if(thread_status != 0)
	{
		printf("\nChild thread - 1 cannot be created\n");
		return 0;
	}

	thread_status = pthread_create(&thread2,&child2_attr,(void *)sender, NULL);
	if(thread_status != 0)
	{
		printf("\nChild thread - 2 cannot be created\n");
		return 0;
	}

	pthread_join(thread1, NULL);
	pthread_join(thread2, NULL);

	return 0;
}