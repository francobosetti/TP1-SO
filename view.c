// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "lib.h"
#include "shmADT.h"

typedef struct {
    char shmName[REGBUFFSIZE];
    char semName[REGBUFFSIZE];
} information;

void prepareData(char * buff, char * fileName, char * md5, int * pidSlave){
    int i=0, j;
    for (j = 0; buff[i]!=','; i++, j++){
        md5[j]=buff[i];
    }
    md5[j] = '\0';
    i++;
    for (j = 0; buff[i]!=','; i++, j++){
        fileName[j]=buff[i];
    }
    fileName[j]='\0';
    i++;
    *pidSlave=atoi(&(buff[i]));
}

void parseData(information * info){
    int val;
    char * buffer = info->shmName;
    int currentPos = 0;
    char c;
    while ( (val = getchar() ) != '\n'){
        c = (char) val;
        if (  (char ) c == ',' ){
            buffer[currentPos] = 0;
            buffer = info->semName;
            currentPos = 0;
        } else {
            buffer[currentPos++] = c;
        }
    }
    buffer[currentPos] = 0;
}

int main(int argc, char *argv[]){
    information info;
    if (argc < 2)
        parseData(&info);
    else {
        strcpy(info.shmName,argv[1]);
        strcpy(info.semName,argv[2]);
    }

    shmADT shareData = openSharedData(info.shmName, info.semName, SHM_SIZE);
    if(shareData==NULL)
        errExitUnlink("Error when opening shared data", shareData);

    char md5[MD5_LENGTH],fileName[MAX_BUFF];
    int pidSlave, bytesRead;
    do {
        if(sem_wait(getSem(shareData))== ERROR)
            errExitUnlink("error at sempahore waiting", shareData);
        char buff[MAX_BUFF];
        if((bytesRead = shmReader(shareData, buff))==ERROR)
            errExitUnlink("Error at trying to read from shared memory", shareData);
        if(bytesRead!=0){
            prepareData(buff, fileName, md5, &pidSlave);
            printf("File name: %s | Hash: %s | SlavePID: %d\n", fileName, md5, pidSlave);
        }
    }
    while(bytesRead != 0);

    if(closeShm(shareData)==ERROR)
        errExit("Error when closing/unlinking");

    return 0;
}
