#include "includes.h"
#include "lib.h"

int main(int argc, char *argv[]){
    //todo tema recibir los datos de la shm
    //todo hacer un semaforo que espere hasta q este habilitado 
    //teniendo los datos de la shm, hay que conectarse a ella y luego para recorrerla usaremos un offset
    
    int fdView= shm_open(SHM_NAME, O_RDWR, S_IRUSR|S_IWUSR);
    void * shmPointer= mmap(NULL, SHM_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fdView, 0);   
    data * buff= (data*) shmPointer;

    int offset=0; //esto se usara para con shm_pointer + offset ir recorriendola y agarrando
    
}