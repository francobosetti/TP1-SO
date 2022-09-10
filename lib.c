#include <stdlib.h>
#include <stdio.h>
#include "lib.h"

void errExit(char * msg){
    perror(msg);
    exit(ERROR);
}
