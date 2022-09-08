#include "lib.h"
#include "shmADT.h"

struct shmCDT{
    sem_t *mutexSem;
    int shmFd;
    int shmSize;
    int currentPos;
    char *shmName;
    char *shmPtr;
    char *semName;
};

int shmWriter(char *buff, shmADT data){
    int size=strlen(buff);

    if((data->currentPos + size + 1) >= data->shmSize)
        return -1;

    for (int i = 0; i < size; i++)
        data->shmPtr[(data->currentPos)++] = buff[i];

    data->shmPtr[(data->currentPos)++]='\n';

    return 0;
}

void getDataFromSHM(shmADT data, char* md5, char* fileName, int * pid){
    for (int i = 0; i < ; ++i) {

    }
}

shmADT initiateSharedData(char * shmName, char * semName, int shmSize) {
    shmADT sharedData = malloc(sizeof(struct shmCDT));
    sharedData->shmName = shmName;
    sharedData->semName = semName;
    sharedData->shmSize=shmSize;
    sharedData->currentPos=0;
    //Just in case there is something at that path I need to unlink it first
    shm_unlink(shmName);
    sem_unlink(semName);

    //SHM CREATION
    sharedData->shmFd=shm_open(shmName, O_CREAT | O_RDWR | O_EXCL, S_IWUSR | S_IRUSR );
    if(sharedData->shmFd==ERROR){
        errExit("shm_open could not be executed");
    }
    if(ftruncate(sharedData->shmFd,sharedData->shmSize)==-1){
        errExit("Space could not be allocated to shared memory segment");
    }
    sharedData->shmPtr = mmap(NULL, sharedData->shmSize, PROT_READ|PROT_WRITE, MAP_SHARED, sharedData->shmFd, 0);
    if(sharedData->shmPtr == MAP_FAILED){
        errExit("Shared memory segment could not be mapped to virtual memory of application process");
    }

    //SEM CREATION
    sharedData->mutexSem = sem_open(semName, O_CREAT |  O_EXCL ,  S_IRUSR| S_IWUSR | S_IROTH| S_IWOTH, 1 );
    if(sharedData->mutexSem==SEM_FAILED){
        errExit("could not create semaphore");
    }
    return sharedData;

}

shmADT openSharedData(char * shmName, char * semName, int shmSize) {
    shmADT sharedData = malloc(sizeof(struct shmCDT));
    sharedData->shmName = shmName;
    sharedData->semName = semName;
    sharedData->shmSize=shmSize;
    //SHM CREATION
    sharedData->shmFd=shm_open(shmName, O_RDWR, S_IWUSR | S_IRUSR );
    if(sharedData->shmFd==-1){
        errExit("shm_open could not be executed");
    }
    char *shmPtr =mmap(NULL, sharedData->shmSize, PROT_READ|PROT_WRITE, MAP_SHARED, sharedData->shmFd, 0);
    if(shmPtr == MAP_FAILED){
        errExit("Shared memory segment could not be mapped to virtual memory of application process");
    }
    sharedData->shmPtr=shmPtr;

    //SEM CREATION
    sharedData->mutexSem = sem_open(semName, 0);
    if(sharedData->mutexSem==SEM_FAILED){
        errExit("could not create semaphore");
    }
    return sharedData;

}

sem_t *getSem(shmADT data){
    return data->mutexSem;
}

char *getShmPtr(shmADT data){
    return data->shmPtr;
}

void unlinkData(shmADT data){
    if(munmap(data->shmPtr, SHM_SIZE)<0){
        errExit("Error executing munmap");
        return;
    }
    if(shm_unlink(data->shmName)<0){
        errExit("Error executing shm_unlink");
        return;
    }
    if(sem_unlink(data->semName)<0){
        errExit("Error executing sem_unlink");
        return;
    }
    free(data);
}

void closeData(shmADT data){
    if(sem_close(data->mutexSem) == -1){
        errExit("Error executing sem_close");
    }
    if(munmap(data->shmPtr, SHM_SIZE) == -1){
        errExit("Error executing munmap");
    }
    if(close(data->shmFd) == -1){
        errExit("Error executing close");
    }
    free(data);
}


