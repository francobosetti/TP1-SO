#ifndef TP1_SO_LIB_H
#define TP1_SO_LIB_H

//Here there will be all the libraries needed for all the files
#define _BSD_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
//libraries for select
#include <sys/select.h>
#include "shmADT.h"
//macros to be used by all .c files

#define SHM_SIZE 256
#define MAX_BUFF 4096
#define MD5_LENGTH 32
#define TRANSFERSIZE 64
#define REGBUFFSIZE 128
#define PIPESIZE 2
#define READPOS 0
#define WRITEPOS 1
#define FINISHREADING 0
#define ERROR (-1)

void errExit(char * msg, shmADT data);

#endif //TP1_SO_LIB_H
