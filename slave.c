#include "includes.h"
#include "lib.h"


void prepareData(data * d,int readFd){
    char buffer[MAX_BUFF];
    ssize_t bytesRead = read(readFd,buffer,MAX_BUFF);
    if ( bytesRead == ERROR){
        errExit("error prepearing data from slave to master");
    }
    //por el formato de md5sum, sale un espacio
    for ( int i = 0; i < bytesRead && buffer[i] != ' '; i++){
        //hacemos deep copy
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


    int md5ToSlave[PIPESIZE];
    if (pipe(md5ToSlave) == ERROR){
        //manejo de error correspondiente
    }

    data d;
    while ( read(STDIN_FILENO,&d, DATA_SIZE) != FINISHREADING){
        int myPid = fork();
        if ( myPid == ERROR ){
            errExit("Error while trying to fork for md5sum in slave");
        }
        if ( myPid == 0) {
            close(md5ToSlave[READPOS]);//md5 no lee, solo escribe
            dup2(md5ToSlave[WRITEPOS],STDOUT_FILENO);
            char * args[] = {"md5sum",d.fileName,NULL};
            //esto ya deberia imprimir a STDOUT, osea que lo agarra papa
            int retVal = execvp(args[0], args);
            if ( retVal < 0){
                errExit("Error while trying to call execv from slave");
            }
        }
        close(md5ToSlave[WRITEPOS]);
        //puede ser que nos interese el valor de retorno, despues lo vemos eso
        wait(NULL);//esperamos al hijo
        prepareData(&d,md5ToSlave[READPOS]);
        sendDataToMaster(&d, STDOUT_FILENO);
    }

}