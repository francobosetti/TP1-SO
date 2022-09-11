#include <stdlib.h>
#include <stdio.h>
#include "lib.h"
#include "shmADT.h"

void errExitUnlink(char * msg, shmADT data){
    perror(msg);
    closeShm(data);
    exit(ERROR);
}

void errExit(char * msg){
    perror(msg);
    exit(ERROR);
}