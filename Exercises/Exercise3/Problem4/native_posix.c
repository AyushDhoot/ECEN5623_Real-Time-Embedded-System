/******************************************************************************************
*	@file		native_posix.c
*	@brief		Pthread FIFO adaptation of VxWorks posix_mq.c
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
#include <sched.h>
#include<sys/syscall.h>

typedef struct Message
{
	char string[200];
#define Q_NAME 		("/my_queue1")
#define Q_SIZE		(8)

	int length;
}mesg_t;

mqd_t msgqueue_FD;
struct mq_attr msgqueue_FD_attr;

void sender(void)
{
	char str[200];
	int value;

	mesg_t message;
	mesg_t *msgptr;

	msgqueue_FD = mq_open(Q_NAME, O_CREAT | O_RDWR, 0666, &msgqueue_FD_attr);	

	if(msgqueue_FD == (mqd_t)-1)
	{
		perror("Error in opening Message Queue");
		exit(1);
	}

	sprintf(str, "%s %ld","this is a test, and only a test, in the event of a real emergency, you would be instructed ... from Task ",syscall(__NR_gettid));
	strcpy(message.string, str);
	message.length = strlen(message.string);
	msgptr = &message;

	value = mq_send(msgqueue_FD, (char*)msgptr, sizeof(mesg_t),30);	
	if(value == -1)
	{
		perror("Error in sending message");
		exit(1);
	}
}

void receiver(void)
{
	int value;
	char str[100];
	int prio;

	mesg_t message;
	mesg_t *msgptr;

	msgqueue_FD = mq_open(Q_NAME, O_CREAT | O_RDWR, 0666, &msgqueue_FD_attr);
	if(msgqueue_FD == -1)
	{
		perror("Error in opennig message queue");
		exit(1);
	}

	msgptr = &message;

	value = mq_receive(msgqueue_FD, (char *)msgptr, sizeof(mesg_t),&prio);
	if(value == -1)
	{
		perror("Error in receiving message");
		exit(1);
	}

	sprintf(str,"%s %ld\n", "Received by Task",syscall(__NR_gettid));
	printf("%s%s, lenght = %d, Priority = %d\n",str,msgptr->string,msgptr->length, prio);
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

	msgqueue_FD_attr.mq_maxmsg = Q_SIZE;
	msgqueue_FD_attr.mq_msgsize = sizeof(mesg_t);

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

	mq_close(msgqueue_FD);
	mq_unlink(Q_NAME);

	return 0;
}
