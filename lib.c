#include <stdlib.h>
#include <stdio.h>
#include "lib.h"
#include "shmADT.h"

void errExit(char * msg, shmADT data){
    perror(msg);
    unlinkData(data);
    exit(ERROR);
}
