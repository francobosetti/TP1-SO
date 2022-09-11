//TODO manejo de errores sin exit, utilizar valores de retorno y errno.h
// https://www.thegeekstuff.com/2010/10/linux-error-codes/ --> setear errExits con valores de ERRNO correspondientes
// y devolver null en caso de error

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

shmADT initiateSharedData(char * shmName, char * semName, int shmSize) {
    shmADT sharedData = malloc(sizeof(struct shmCDT));
    sharedData->shmName = shmName;
    sharedData->semName = semName;
    sharedData->shmSize = shmSize;
    sharedData->currentPos=0;

    //Just in case there is something at that path I need to unlink it first
    shm_unlink(shmName);
    sem_unlink(semName);

    //SHM CREATION
    sharedData->shmFd=shm_open(shmName, O_CREAT | O_RDWR | O_EXCL, S_IWUSR | S_IRUSR );
    if(sharedData->shmFd==ERROR){
        return NULL;
    }

    if(ftruncate(sharedData->shmFd,sharedData->shmSize)==ERROR){
        return NULL;
    }

    sharedData->shmPtr = mmap(NULL, sharedData->shmSize, PROT_READ|PROT_WRITE, MAP_SHARED, sharedData->shmFd, 0);
    if(sharedData->shmPtr == MAP_FAILED){
        return NULL;
    }
    //SEM CREATION
    sharedData->mutexSem = sem_open(semName, O_CREAT |  O_EXCL ,  S_IRUSR| S_IWUSR | S_IROTH| S_IWOTH, 1);
    if(sharedData->mutexSem==SEM_FAILED){
        return NULL;
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
    if(sharedData->shmFd==ERROR){
        return NULL;
    }
    char *shmPtr = mmap(NULL, sharedData->shmSize, PROT_READ|PROT_WRITE, MAP_SHARED, sharedData->shmFd, 0);
    if(shmPtr == MAP_FAILED){
        return NULL;
    }
    sharedData->shmPtr=shmPtr;

    //SEM CREATION
    sharedData->mutexSem = sem_open(semName, 0);
    if(sharedData->mutexSem==SEM_FAILED){
        return NULL;
    }
    return sharedData;
}

// Returns a negative value if an error was encountered, app must handle it
int shmWriter(shmADT data, char * buff){
    if(data == NULL || buff == NULL)
        return -1; //App hara el manejo de error

    int bytesWritten;

    bytesWritten = sprintf(&(data->shmPtr[data->currentPos]), "%s", buff);

    if(bytesWritten > 0)
        data->currentPos += bytesWritten + 1;

    return bytesWritten;
}

//Returns qty of bytes read, error handling must be done by calling process
int shmReader(shmADT data, char * buff){
    if(data == NULL || buff == NULL)
        return -1; //View hara el manejo de error

    int bytesRead;

    bytesRead = sprintf(buff, "%s", &(data->shmPtr[data->currentPos]));

    if(bytesRead > 0)
        data->currentPos += bytesRead + 1;

    return bytesRead;
}

int unlinkData(shmADT data){
    if(data == NULL || munmap(data->shmPtr, SHM_SIZE)<0 || shm_unlink(data->shmName)<0 || sem_unlink(data->semName)<0)
        return -1;
    free(data);
    return 0;
}

int closeData(shmADT data){
    if(sem_close(data->mutexSem) == ERROR || munmap(data->shmPtr, SHM_SIZE) == ERROR || close(data->shmFd) == ERROR)
        return -1;
    free(data);
    return 0;
}

sem_t *getSem(shmADT data){
    return data->mutexSem;
}

