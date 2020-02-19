#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include <semaphore.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <fcntl.h>

#define SEM     "my_semaphore"

sem_t *sem;

void main()
{
   sem = sem_open(SEM, O_CREAT, 0644, 0);
   printf("Created a new process\n");
   printf("Waiting for a semaphore\n");
   sem_wait(sem);
   printf("Unlocked. Now resume execution\n");
   sem_destroy(sem);
}