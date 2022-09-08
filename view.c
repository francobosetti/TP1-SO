#include "lib.h"
#include "shmADT.h"

// Hash: ... | Nombre: ... | Slave PID: ...\n
int main(int argc, char *argv[]){
    //todo tema recibir los datos de la shm
    //todo hacer un semaforo que espere hasta q este habilitado 
    shmADT shareData= openSharedData(SHM_NAME, SEM_NAME, SHM_SIZE);

    char md5[MD5_LENGTH],fileName[MAX_BUFF];
    int pidSlave;
    while(...){
        if(sem_wait(getSem(shareData))== ERROR)
            errExit("error at sempahore waiting");
        getDataFromSHM(shareData, md5, fileName, &pidSlave);
        printf("File name:%s, md5sum: %s, SlavePID: %d", fileName,md5, pidSlave);
    }
}