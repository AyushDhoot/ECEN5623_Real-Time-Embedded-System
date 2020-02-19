
#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include <semaphore.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <fcntl.h>

#define SEM  "my_semaphore"

sem_t *sem;

void main()
{
   sem = sem_open(SEM, 0);
   printf("Created a process for unlock\n");
   sem_post(sem);
   printf("Process has now resumed\n");
}

