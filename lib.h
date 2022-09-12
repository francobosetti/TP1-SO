#ifndef TP1_SO_LIB_H
#define TP1_SO_LIB_H

#define _BSD_SOURCE
#define _XOPEN_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>

//libraries for select
#include <sys/select.h>
#include "shmADT.h"

//Macros
#define MAX_BUFF 4096
#define MD5_LENGTH 32
#define TRANSFERSIZE 64
#define REGBUFFSIZE 128
#define PIPESIZE 2
#define READPOS 0
#define WRITEPOS 1
#define FINISHREADING 0


void errExit(char * msg);
void errExitUnlink(char * msg, shmADT data);

#endif //TP1_SO_LIB_H
