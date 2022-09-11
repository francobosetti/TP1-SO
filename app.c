#include "lib.h"
#include "shmADT.h"

//we have to define number of child that the parent will create and how many tasks can a child make at MAX
// _BSD-SOURCE --> VER BIEN ESTO, ES PARA LA MACRO ISDIR
#define NUM_CHILDS 4
#define MAX_TASK_PER_CHILD 3
#define MAX_SLAVE_OUTPUT 256
#define CHILDTASKS 2

#define SLAVE_NAME "./slave"
#define ANSWER_NAME "./Answers.txt"

typedef struct slaveComm{
    int masterToSlaveFd[PIPESIZE];
    int slaveToMasterFd[PIPESIZE];
}slaveComm;


//devuelve como valor de retorno el maximo fileDescriptor

int createReadSet(slaveComm * comms, fd_set * set){
    FD_ZERO(set);
    int max;
    for (int i = 0; i < NUM_CHILDS; ++i){
        //trabajoo con slaveToMaster
        FD_SET(comms[i].slaveToMasterFd[READPOS], set);
        if ( i == 0 || comms[i].slaveToMasterFd[READPOS] > max)
            max = comms[i].slaveToMasterFd[READPOS];
    }
    return max;
}

//devuelve como valor de retorno el maximo fileDescriptor
int createWriteSet(slaveComm * comms, fd_set * set){
    FD_ZERO(set);
    int max;
    for (int i = 0; i < NUM_CHILDS; ++i){
        //trabajoo con masterToSlave
        FD_SET(comms[i].masterToSlaveFd[WRITEPOS], set);
        if ( i == 0 || comms[i].masterToSlaveFd[WRITEPOS] > max)
            max = comms[i].masterToSlaveFd[WRITEPOS];
    }
    return max;
}

void sendDataToFile(FILE * fptr, char * buffer){
    fprintf(fptr,"%s",buffer);
}

void prepareHeaders(FILE * fptr){
    fprintf(fptr,"Hash,FileName,slavePid\n");
}

int getMax(int num1, int num2){
    return num1 > num2 ? num1:num2;
}

int isReg(const char* fileName){
    struct stat path;
    stat(fileName, &path);
    return S_ISREG(path.st_mode);
}


void sendTask(slaveComm *comm, char * file){
    if(write(comm->masterToSlaveFd[WRITEPOS], file, strlen(file)) == ERROR)
        errExit("Failed at writing on child pipe from master");
}

void sendTaskToChild(char * fileName, slaveComm * comm){
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
    
    //chequeo si estoy escribiendo en un pipe
    struct stat s;
    fstat(STDOUT_FILENO,&s);
    if ( S_ISFIFO(s.st_mode) ) {
        printf("%s,%s\n", SHM_NAME, SEM_NAME);
    }

    //Creating the semaphore and shm
    shmADT shareData = initiateSharedData(SHM_NAME, SEM_NAME, SHM_SIZE);
    if(shareData==NULL)
        errExit("Error when initiating shared data");
    

    sleep(2);

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
            int retVal = execv(SLAVE_NAME,args);
            if ( retVal == ERROR)
                errExit("Error in execv syscall");
        }
        //master no escribe en slaveToMaster y no lee en masterToSlave
        close(communications[i].slaveToMasterFd[WRITEPOS]);
        close(communications[i].masterToSlaveFd[READPOS]);
    }


    int cantFiles = argc - 1;
    int cantNoRegFiles = 0;

    FILE * resultFile = fopen("results.csv","w+");
    prepareHeaders(resultFile);

    for ( int filesProccesed = 0, currentFile = 1; filesProccesed < cantFiles - cantNoRegFiles ;){
        fd_set readSet, writeSet;
        int maxfd = getMax(createReadSet(communications,&readSet),createWriteSet(communications,&writeSet));

        if (select(maxfd + 1, &readSet, &writeSet,NULL,NULL) == ERROR)
            errExit("error while using select");
        

        //chequear lo de ready con EOF
        for (int i = 0; i < NUM_CHILDS && argv[currentFile] != NULL; currentFile++, i++){
            if (!FD_ISSET(communications[i].masterToSlaveFd[WRITEPOS],&writeSet))
                continue;
            if (!isReg(argv[currentFile])){
                cantNoRegFiles++;
                continue;
            }
            //si puedo escribir en este fd
            sendTaskToChild(argv[currentFile],&communications[i]);
        }

        for ( int i = 0; i < NUM_CHILDS ; i++) {
            if (!FD_ISSET(communications[i].slaveToMasterFd[READPOS], &readSet))
                continue;
            //si puedo leer en este fd
            char buffer[MAX_BUFF];
            getData(buffer, communications[i].slaveToMasterFd[READPOS]);
            sendDataToFile(resultFile,buffer);
            shmWriter(shareData, buffer);
            sem_post(getSem(shareData));
            filesProccesed++;
        }
    }

    fclose(resultFile);
    //TODO el pipe no hace que corran en simultaneo, se elimina el shm antes de ejecutarse el view
    //unlinkData(shareData);

    return 0;
}
