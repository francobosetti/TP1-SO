#include "lib.h"
#include "shmADT.h"



typedef struct {
    char shmName[REGBUFFSIZE];
    char semName[REGBUFFSIZE];
} information;

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
    *pidSlave=atoi(buff[i]);
}



void parseData(information * info){
    
}


// orden es shmName,semName
int main(int argc, char *argv[]){
    //todo tema recibir los datos de la shm
    //todo hacer un semaforo que espere hasta q este habilitado

    if ( argc < 2){
        //entonces no tengo los datos en argv


    } else {
        //si tengo los datos en argv

    }


    

    shmADT shareData= openSharedData(shmName, semName, SHM_SIZE);
    if(shareData==NULL)
        errExit("Error when opening shared data");

    char md5[MD5_LENGTH],fileName[MAX_BUFF];
    int pidSlave;
    while(i <= cantTask){
        if(sem_wait(getSem(shareData))== ERROR)
            errExit("error at sempahore waiting");
        char buff[MAX_BUFF];
        if(shmReader(shareData, buff)==ERROR)
            errExit("Error at trying to read from shared memory");
        prepareData(buff, fileName, md5, &pidSlave);
        printf("File name:%s, md5sum: %s, SlavePID: %d", fileName, md5, pidSlave);
    }
}