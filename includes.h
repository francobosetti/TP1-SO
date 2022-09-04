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

