#include "lib.h"
#include "shmADT.h"

int getCantDigits(int n){
    int count = 0;
    do {
        n /= 10;
        ++count;
    } while (n != 0);
    return count;
}

void prepareData(char * buff, char * fileName, char * md5, int * pidSlave){
    int i=0;
    for (int j = 0; buff[i]!=","; i++, j++)
    {
        md5[j]=buff[i];
    }
    i++;
    for (int j = 0; buff[i]!=","; i++, j++)
    {
        fileName[j]=buff[i];
    }
    i++;
    &pidSlave=atoi(buff[i]);
}

void getDataFromSHM(char * shmPtr, char* md5, char* fileName, int * pid){
    for (int i = 0; PARSER(data->shmPtr[data->currentPos]); ++i, (data->currentPos)++)
        md5[i] = data->shmPtr[data->currentPos];

    for (int i = 0; PARSER(data->shmPtr[data->currentPos]); ++i, (data->currentPos)++)
        fileName[i] = data->shmPtr[data->currentPos];

    (*pid) = atoi(data->shmPtr+data->currentPos);
    data->currentPos += getCantDigits(*pid);
}

// Hash: ... | Nombre: ... | Slave PID: ...\n
int main(int argc, char *argv[]){
    //todo tema recibir los datos de la shm
    //todo hacer un semaforo que espere hasta q este habilitado 
    shmADT shareData= openSharedData(SHM_NAME, SEM_NAME, SHM_SIZE);
    if(shareData==NULL)
        errExit("Error when opening shared data");

    char md5[MD5_LENGTH],fileName[MAX_BUFF];
    int pidSlave;
    while(...){
        if(sem_wait(getSem(shareData))== ERROR)
            errExit("error at sempahore waiting");
        char buff[MAX_BUFF];
        shmReader(shareData, buff);
        prepareData(buff, fileName, md5, &pidSlave);
        printf("File name:%s, md5sum: %s, SlavePID: %d", fileName, md5, pidSlave);
    }
}