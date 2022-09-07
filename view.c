#include "includes.h"
#include "lib.h"

int main(int argc, char *argv[]){
    //todo tema recibir los datos de la shm
    //todo hacer un semaforo que espere hasta q este habilitado 
    sem_t * semaphore=sem_open(SEM_NAME, 0); //todo revisar si hay que agregarle los otros 2 parametros con el utimo siendo 0
    if(semaphore==SEM_FAILED)
        errExit("Could not open sempahore");
    
    //teniendo los datos de la shm, hay que conectarse a ella y luego para recorrerla usaremos un offset

    int fdView= shm_open(SHM_NAME, O_RDWR, S_IRUSR|S_IWUSR);
    void * shmPointer= mmap(NULL, SHM_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fdView, 0);   
    data * buff= (data*) shmPointer;

    int offset=0; //esto se usara para con shm_pointer + offset ir recorriendola y agarrando
    while(...){
        if(sem_wait(semaphore)== ERROR)
            errExit("error at sempahore waiting");
        printf("File name:%s, md5sum: %s, SlavePID: %d", (buff+offset)->fileName,(buff+offset)->md5, (buff+offset)->pidSlave);
        offset+=DATA_SIZE;
    }
}