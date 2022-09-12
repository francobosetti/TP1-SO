// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
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