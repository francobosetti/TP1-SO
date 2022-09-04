//Here there will be all the libraries needed for all the files
#define _BSD_SOURCE 

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
       
//libraries for semaphores
#include <semaphore.h>
#include <sys/wait.h>
#include <signal.h>


//libraries for shared memory
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */
#include <sys/types.h>
#include <sys/mman.h>

//libraries for select
#include <sys/select.h>


//macros to be used by all .c files
#define SHM_NAME "/appshm"
#define SHM_SIZE 1024
#define MAX_BUFF 256
#define MD5_LENGTH 32

typedef struct data{
    char fileName[MAX_BUFF];
    char md5[MD5_LENGTH];
    pid_t pidSlave;
}data;

#define DATA_SIZE (sizeof(data))
