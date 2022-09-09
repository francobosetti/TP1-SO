#ifndef TP1_SO_SHMADT_H
#define TP1_SO_SHMADT_H

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>

//libraries for shared memory
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */
#include <sys/types.h>
#include <sys/mman.h>

//libraries for semaphores
#include <semaphore.h>
#include <sys/wait.h>
#include <signal.h>

#define _BSD_SOURCE
#define SHM_NAME "/appshm"
#define SEM_NAME "/sem"
#define SHM_SIZE 256

#define ERROR -1

typedef struct shmCDT * shmADT;
shmADT initiateSharedData(char * shmName, char * semName, int shmSize);
shmADT openSharedData(char * shmName, char * semName, int shmSize);
sem_t *getSem(shmADT data);
char *getShmPtr(shmADT data);
void closeData(shmADT data);
void unlinkData(shmADT data);
int shmWriter(shmADT data, char *buff);
int shmReader(shmADT data, char * buff);
#endif //TP1_SO_SHMADT_H
