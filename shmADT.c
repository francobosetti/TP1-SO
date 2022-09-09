//TODO manejo de errores sin exit, utilizar valores de retorno y errno.h
// https://www.thegeekstuff.com/2010/10/linux-error-codes/ --> setear errExits con valores de ERRNO correspondientes
// y devolver null en caso de error

#include "shmADT.h"

#define PARSER(n) ((n) != ',' && (n) != '\n')

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

// Returns a negative value if an error was encountered, app must handle it
int shmWriter(shmADT data, char * buff){
    if(data == NULL || buff == NULL)
        return -1; //App hara el manejo de error

    int bytesWritten;

    bytesWritten = sprintf(&(data->shmPtr[data->currentPos]), "%s", buff);

    if(bytesWritten > 0)
        data->currentPos += bytesWritten + 1;

    return bytesWritten;

    /*
    int size=strlen(buff);

    if((data->currentPos + size + 1) >= data->shmSize)
        return -1;


    for (int i = 0; i < size; i++)
        data->shmPtr[(data->currentPos)++] = buff[i];

    data->shmPtr[(data->currentPos)++]='\n';

     return 0;
     */

}

//Returns qty of bytes read, error handling must be done by calling process
int shmReader(shmADT data, char * buff){
    if(data == NULL || buff == NULL)
        return -1; //App hara el manejo de error

    int bytesRead;

    bytesRead = sprintf(buff, "%s", &(data->shmPtr[data->currentPos]));

    if(bytesRead > 0)
        data->currentPos += bytesRead + 1;

    return bytesRead;
}

//todo cambiar exit
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

//todo cambiar exit
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


