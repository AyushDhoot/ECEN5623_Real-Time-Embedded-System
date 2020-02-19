#define _GNU_SOURCE
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <sched.h>
#include <time.h>
#include <unistd.h>
#include <semaphore.h>

#define NUM_THREADS					(2)

#define NSEC_PER_SEC (1000000000)
#define NSEC_PER_MSEC (1000000)
#define NSEC_PER_MICROSEC (1000)
#define DELAY_TICKS (1)
#define ERROR (-1)
#define OK (0)


useconds_t delay_20ms = 20000, delay_10ms = 10000;

unsigned int idx = 0, jdx = 1;
unsigned int seqIterations = 47;
unsigned int reqIterations = 128500;
volatile unsigned int fib = 0, fib0 = 0, fib1 = 1;




#define FIB_TEST(seqCnt, iterCnt)      \
   for(idx=0; idx < iterCnt; idx++)    \
   {                                   \
      fib = fib0 + fib1;               \
      while(jdx < seqCnt)              \
      {                                \
         fib0 = fib1;                  \
         fib1 = fib;                   \
         fib = fib0 + fib1;            \
         jdx++;                        \
      }                                \
   }                                   \


typedef struct
{
    int threadIdx;
} threadParams_t;

// POSIX thread declarations and scheduling attributes

pthread_t threads[NUM_THREADS];
threadParams_t threadParams[NUM_THREADS];
pthread_attr_t rt_sched_attr[NUM_THREADS];
int rt_max_prio, rt_min_prio;
struct sched_param rt_param[NUM_THREADS];
struct sched_param seq_param;
pid_t mainpid;

//Semaphore declaration
sem_t sem_fib10, sem_fib20;

//Global variable to indicate test completition
int fib10_flag = 0, fib20_flag = 0;

/*Start time*/
struct timespec start_time = {0, 0};


int delta_t(struct timespec *stop, struct timespec *start, struct timespec *delta_t)
{
  int dt_sec=stop->tv_sec - start->tv_sec;
  int dt_nsec=stop->tv_nsec - start->tv_nsec;

  if(dt_sec >= 0)
  {
    if(dt_nsec >= 0)
    {
      delta_t->tv_sec=dt_sec;
      delta_t->tv_nsec=dt_nsec;
    }
    else
    {
      delta_t->tv_sec=dt_sec-1;
      delta_t->tv_nsec=NSEC_PER_SEC+dt_nsec;
    }
  }
  else
  {
    if(dt_nsec >= 0)
    {
      delta_t->tv_sec=dt_sec;
      delta_t->tv_nsec=dt_nsec;
    }
    else
    {
      delta_t->tv_sec=dt_sec-1;
      delta_t->tv_nsec=NSEC_PER_SEC+dt_nsec;
    }
  }

  return(1);
}

void *fib10(void *threadp)
{
  
    int sum=0, i;
    pthread_t thread;
    cpu_set_t cpuset;
    
   
    thread=pthread_self();
    CPU_ZERO(&cpuset);
   
    struct timespec finish_time = {0, 0};
    struct timespec thread_dt = {0, 0};
    threadParams_t *threadParams = (threadParams_t *)threadp;

/*
    pthread_getaffinity_np(thread, sizeof(cpu_set_t), &cpuset);
        printf("Thread idx=%d, affinity contained:", threadParams->threadIdx);
        for(i=0; i<4; i++)
            if(CPU_ISSET(i, &cpuset))  printf(" CPU-%d ", i);
*/

    while(!fib10_flag){
   
    sem_wait(&sem_fib10);

    // COMPUTE SECTION
    for(i=1; i < (threadParams->threadIdx)+1; i++)    
		sum=sum+i;
		
    //for(i=1; i <=7; i++)
    FIB_TEST(seqIterations, reqIterations);
    // END COMPUTE SECTION

    clock_gettime(CLOCK_REALTIME, &finish_time);
    delta_t(&finish_time, &start_time, &thread_dt);
    
    printf("\nThread idx=%d with priority = %d ran %ld sec, %ld msec (%ld microsec)\n", threadParams->threadIdx, rt_param[0].sched_priority, thread_dt.tv_sec, 
            (thread_dt.tv_nsec / NSEC_PER_MSEC), (thread_dt.tv_nsec / NSEC_PER_MICROSEC));
  }
  
   
}

void *fib20(void *threadp)
{
    
    int sum=0, i;
    pthread_t thread;
     cpu_set_t cpuset;
    
       
    thread=pthread_self();
    CPU_ZERO(&cpuset);

    struct timespec finish_time = {0, 0};
    struct timespec thread_dt = {0, 0};
    threadParams_t *threadParams = (threadParams_t *)threadp;
    
    /*
    pthread_getaffinity_np(thread, sizeof(cpu_set_t), &cpuset);
        printf("Thread idx=%d, affinity contained:", threadParams->threadIdx);
        for(i=0; i<4; i++)
            if(CPU_ISSET(i, &cpuset))  printf(" CPU-%d ", i);
    */
    
    while(!fib20_flag){

    sem_wait(&sem_fib20);

    // COMPUTE SECTION
    for(i=1; i < (threadParams->threadIdx)+1; i++)
        sum=sum+i;

    //for(i=1; i <=2; i++)
		FIB_TEST(seqIterations, (2*reqIterations));
    // END COMPUTE SECTION 

    clock_gettime(CLOCK_REALTIME, &finish_time);
    delta_t(&finish_time, &start_time, &thread_dt);
    
    printf("\nThread idx=%d with priority = %d ran %ld sec, %ld msec (%ld microsec)\n", threadParams->threadIdx, rt_param[1].sched_priority ,thread_dt.tv_sec, 
              (thread_dt.tv_nsec / NSEC_PER_MSEC), (thread_dt.tv_nsec / NSEC_PER_MICROSEC));
  }
  
}

void print_scheduler(void)
{
   int schedType;

   schedType = sched_getscheduler(getpid());

   switch(schedType)
   {
     case SCHED_FIFO:
           printf("Pthread Policy is SCHED_FIFO\n");
           break;
     case SCHED_OTHER:
           printf("Pthread Policy is SCHED_OTHER\n");
       break;
     case SCHED_RR:
           printf("Pthread Policy is SCHED_OTHER\n");
           break;
     default:
       printf("Pthread Policy is UNKNOWN\n");
   }

}

int main(int argc, char const *argv[])
{
   int rc;
   int i, scope;
   cpu_set_t allcpuset;
   cpu_set_t threadcpu;
   
	CPU_ZERO(&allcpuset);
    CPU_SET(0, &allcpuset);

   clock_gettime(CLOCK_REALTIME, &start_time);

   sem_init(&sem_fib20, 0, 1);
   sem_init(&sem_fib10, 0, 1);

   rt_max_prio = sched_get_priority_max(SCHED_FIFO);
   rt_min_prio = sched_get_priority_min(SCHED_FIFO);

   print_scheduler();
   rc=sched_getparam(mainpid, &seq_param);
   seq_param.sched_priority=rt_max_prio;
   rc=sched_setscheduler(getpid(), SCHED_FIFO, &seq_param);
   if(rc < 0) perror("seq_param");
   print_scheduler();

   for(i=0; i < NUM_THREADS; i++)
   {
	   
	   CPU_SET(0, &threadcpu);
	   
       rc=pthread_attr_init(&rt_sched_attr[i]);
       rc=pthread_attr_setinheritsched(&rt_sched_attr[i], PTHREAD_EXPLICIT_SCHED);
       rc=pthread_attr_setschedpolicy(&rt_sched_attr[i], SCHED_FIFO);
	   rc=pthread_attr_setaffinity_np(&rt_sched_attr[i], sizeof(cpu_set_t), &threadcpu);
       
       rt_param[i].sched_priority=rt_max_prio-i-1;
       pthread_attr_setschedparam(&rt_sched_attr[i], &rt_param[i]);

       threadParams[i].threadIdx=i;
   }


    
    pthread_create(&threads[0],   // pointer to thread descriptor
			&rt_sched_attr[0],          // use default attributes
			fib10,                      // thread function entry point
			(void *)&(threadParams[0])  // parameters to pass in
			);


    pthread_create(&threads[1],     // pointer to thread descriptor
			&rt_sched_attr[1],              // use default attributes
			fib20,                          // thread function entry point
			(void *)&(threadParams[1])      // parameters to pass in
      );

  
    usleep(delay_20ms); 
    sem_post(&sem_fib10);
    usleep(delay_20ms); 
    sem_post(&sem_fib10);
    usleep(delay_10ms); 
    
    fib20_flag = 1;
    
    sem_post(&sem_fib20);
    usleep(delay_10ms); 
    sem_post(&sem_fib10);
    usleep(delay_20ms); 
    
    fib10_flag = 1;
    
    sem_post(&sem_fib10);
    usleep(delay_20ms); 
      
    
    for(i=0;i<NUM_THREADS;i++)
        pthread_join(threads[i], NULL);

 
    pthread_exit(NULL);

    printf("\nTEST COMPLETE\n");

	return 0;
}
CC=gcc


CFLAGS= -O0 -g 
  
CFILES= linux_implementation.c

SRCS= ${CFILES}
OBJS= ${CFILES:.c=.o}

all:	linux_implementation

clean:
	-rm -f *.o *.d
	-rm -f linux_implementation

linux_implementation : linux_implementation.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $@.o -lpthread -lrt

depend:

.c.o:
	$(CC) $(CFLAGS) -c $<
