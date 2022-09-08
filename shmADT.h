#ifndef TP1_SO_SHMADT_H
#define TP1_SO_SHMADT_H

#define _BSD_SOURCE
#define SHM_NAME "/appshm"
#define SEM_NAME "/sem"
#define SHM_SIZE 256

typedef struct shmCDT * shmADT;
shmADT initiateSharedData(char * shmName, char * semName, int shmSize);
shmADT openSharedData(char * shmName, char * semName, int shmSize);
sem_t *getSem(shmADT data);
char *getShmPtr(shmADT data);
void closeData(shmADT data);
void unlinkData(shmADT data);
int shmWriter(shmADT data, char *buff);
#endif //TP1_SO_SHMADT_H
