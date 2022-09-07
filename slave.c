#include "includes.h"
#include "lib.h"


void prepareData(data * d,char * buffer){
    for ( int i = 0 ; buffer[i] != 0 && buffer[i] != ' ' ; i++){
        d->md5[i] = buffer[i];
    }
}

void sendDataToMaster(data * d, int writeFd){
    pid_t myPid = getpid();
    //TOOD manejo de error del pid
    d->pidSlave = myPid;
    write(writeFd, &d, DATA_SIZE);
}

int main(void){
    //desabilitamos para flushing
    if(setvbuf(stdout, NULL, _IONBF, 0)!=0){
        perror("setvbuf failed/n");
    }

    data d;
    char buffer[MAX_BUFF];
    while ( read(STDIN_FILENO,&d, DATA_SIZE) != FINISHREADING){
        //entonces concateno direccion y comando con sprintf
        sprintf(buffer,"md5sum %s",d.fileName);

        File * f = popen(buffer,"r");
        if ( f == NULL )
            errExit("Error while using popen from slave");

        //ver posibles errores con fgets
        fgets(buffer,MAX_BUFF,f);
        prepareData(&d, buffer);
        sendDataToMaster(&d,STDOUT_FILENO);
        int closeVal = pclose(f);
        if (pclose == ERROR)
            errExit("a problem occured while closing md5sum command");
        
    }

}