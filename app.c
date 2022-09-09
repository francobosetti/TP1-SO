#include "lib.h"
#include "shmADT.h"

//we have to define number of child that the parent will create and how many tasks can a child make at MAX
// _BSD-SOURCE --> VER BIEN ESTO, ES PARA LA MACRO ISDIR
#define NUM_CHILDS 4
#define MAX_TASK_PER_CHILD 3
#define MAX_SLAVE_OUTPUT 256

#define SLAVE_NAME "./slave"
#define ANSWER_NAME "./Answers.txt"

typedef struct slaveComm{
    int masterToSlaveFd[PIPESIZE];
    int slaveToMasterFd[PIPESIZE];
}slaveComm;

int isReg(const char* fileName){
    struct stat path;
    stat(fileName, &path);
    return S_ISREG(path.st_mode);
}

void createSetAndMaxFd(slaveComm *comms, fd_set * set, int * maxFd){
    FD_ZERO(set);
    *maxFd = comms[0].slaveToMasterFd[READPOS];
    for (int i = 0; i < NUM_CHILDS; ++i){
        FD_SET(comms[i].slaveToMasterFd[READPOS], set);

        if(comms[i].slaveToMasterFd[READPOS] > *maxFd)
            *maxFd = comms[i].slaveToMasterFd[READPOS];
    }
}

void sendTask(slaveComm *comm, char * file){
    if(write(comm->masterToSlaveFd[WRITEPOS], file, strlen(file)) == ERROR)
        errExit("Failed at writing on child pipe from master");
}

void queueIfFile(char * fileName, slaveComm * comm){
    if ( !isReg(fileName) )
        return;
    //si el archivo es regular lo mandamos

    char aux[TRANSFERSIZE];
    sprintf(aux,"%s\n",fileName);
    sendTask(comm,aux);
}

void getData(char * buffer, int fd){
    char c;
    bool foundNewLine = false;
    int i;
    for (i = 0; !foundNewLine && read(fd, &c, 1) > 0; ++i){
        buffer[i] = c;
        if ( c == '\n')
            foundNewLine = true;
    }
    buffer[i] = 0;
}

int main(int argc, char *argv[]){

    if(argc<2)
        errExit("Application process did not recieve enough arguments");
    
    //Creating the semaphore and shm
    shmADT shareData = initiateSharedData(SHM_NAME, SEM_NAME, SHM_SIZE);
    if(shareData==NULL)
        errExit("Error when initiating shared data");
    
/*
    sleep(2);   //todo la consigna dice Cuando inicia, DEBE esperar 2 segundos a que aparezca un
    //proceso vista, si lo hace le comparte el buffer de llegada.
    //vector de comunicacion para pipes
    */
    slaveComm communications[NUM_CHILDS];

    //inicializacion de procesos esclavos
    for (int i = 0; i < NUM_CHILDS ; i++){
        if (pipe(communications[i].masterToSlaveFd) == ERROR)
            errExit("Error in master to slave pipe creation");
        if ( pipe(communications[i].slaveToMasterFd) == ERROR)
            errExit("Error in slave to master pipe creation");

        pid_t  myPid =  fork();
        if(myPid == ERROR)
            errExit("Fork could not be executed");
        if (myPid == 0){
            //tenemos que dejar el pipe bien hecho
            close(communications[i].masterToSlaveFd[WRITEPOS]);//slave no escribe en masterToSlave
            close(communications[i].slaveToMasterFd[READPOS]);//slave no lee en slaveToMaster

            dup2(communications[i].slaveToMasterFd[WRITEPOS],STDOUT_FILENO);//slave escribe a entrada de slaveToMaster
            close(communications[i].slaveToMasterFd[WRITEPOS]);

            dup2(communications[i].masterToSlaveFd[READPOS],STDIN_FILENO);
            close(communications[i].masterToSlaveFd[READPOS]);

            char * args[] = {NULL};
            execv(SLAVE_NAME,args);
        }
        //master no escribe en slaveToMaster y no lee en masterToSlave
        close(communications[i].slaveToMasterFd[WRITEPOS]);
        close(communications[i].masterToSlaveFd[READPOS]);
    }

    //primero se cargan todos los programas posibles en los h
    //primero se cargan todos los programas posibles en los hijos
    int currentFile=1;
    for (int i=0; argv[currentFile]!=NULL && i<NUM_CHILDS; i++ , currentFile++) {
        queueIfFile(argv[currentFile], &communications[i]);
    }

    //el primer argumento es el nombre del programa
    for (int filesProcessed = 0; filesProcessed < argc - 1;)   {
        fd_set set;
        int maxFd;
        createSetAndMaxFd(communications, &set, &maxFd);

        if(select(maxFd + 1,&set, NULL, NULL,NULL) == ERROR)
            errExit("Failed when using select");
        //Check which fd is ready
        for (int i = 0; i < NUM_CHILDS; i++) {
            if(FD_ISSET(communications[i].slaveToMasterFd[READPOS], &set)){
                char buffer[MAX_SLAVE_OUTPUT];
                getData(buffer, communications[i].slaveToMasterFd[READPOS]);
                //shmWriter(buffer, shareData);
                puts(buffer);
                filesProcessed++;
                //if(sem_post(getSem(shareData))==ERROR)
                //errExit("could not execute sem_post");
                //TODO fijarse que si es directorio, se sale del queueIfFile y no entra nada a ese slave
                queueIfFile(argv[currentFile], &communications[i]);
                //TODO HAY ALGO RARO CON EL INCREMENTO DE CURRENTFILE
                currentFile++;
            }
        }

    }

}
