#include "includes.h"
#include "lib.h"
#include <stdbool.h>

//we have to define number of child that the parent will create and how many tasks can a child make at MAX
// _BSD-SOURCE --> VER BIEN ESTO, ES PARA LA MACRO ISDIR
#define NUM_CHILDS 4
#define MAX_TASK_PER_CHILD 3

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

fd_set createSet(slaveComm *comms){
    fd_set readSet;
    FD_ZERO(&readSet);
    for (int i = 0; i < NUM_CHILDS; ++i)
        FD_SET(comms[i].slaveToMasterFd[READPOS], &readSet);
    return readSet;
}

void sendTask(slaveComm comm, char * fileName){
    data d;
    strcpy(d.fileName, fileName);
    if(write(comm.masterToSlaveFd[WRITEPOS], &d, DATA_SIZE) == ERROR)
        errExit("Failed at writing on child pipe from master");
}

void queueIfFile(char * argv[], slaveComm comm, int * currentFile){
    bool taskSent = false;
    while(argv[*currentFile] != NULL && !taskSent){
        if(!isReg(argv[*currentFile])) {
            sendTask(comm, argv[*currentFile]);
            taskSent = true;
        }
        (*currentFile)++;
    }
}

int main(int argc, char *argv[]){

    if(argc<2)
        errExit("Application process did not recieve enough arguments");

    //Creating the semaphore 
    sem_t * semaphore= sem_open(SEM_NAME, O_CREAT | O_EXCL, S_IRUSR| S_IWUSR | S_IROTH| S_IWOTH, 1);
    if(semaphore== SEM_FAILED)
        errExit("could not create semaphore");

    //Creating the shared memory segment
    int fdParent = shm_open(SHM_NAME, O_CREAT|O_RDWR, S_IRUSR|S_IWUSR);
    if(fdParent == -1)
        errExit("shm_open could not be executed");

    if(ftruncate(fdParent, SHM_SIZE * DATA_SIZE))
        errExit("Space could not be allocated to shared memory segment");

    void * shmPointer = mmap(NULL, SHM_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fdParent, 0);
    if(shmPointer == MAP_FAILED)
        errExit("Shared memory segment could not be mapped to virtual memory of application process");

    //Circular vector to be used by multiple processes
    data * buff = (data *) shmPointer;

    sleep(2);   //todo la consigna dice Cuando inicia, DEBE esperar 2 segundos a que aparezca un 
                //proceso vista, si lo hace le comparte el buffer de llegada.

    //vector de comunicacion para pipes
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
            dup2(communications[i].masterToSlaveFd[READPOS],STDIN_FILENO);
            char * args[] = {NULL};
            execv(SLAVE_NAME,args);
        }
        //master no escribe en slaveToMaster y no lee en masterToSlave
        close(communications[i].slaveToMasterFd[WRITEPOS]);
        close(communications[i].masterToSlaveFd[READPOS]);
    }

    //primero se cargan todos los programas posibles en los hijos
    int currentFile=1;
    for (int i=0; argv[currentFile]!=NULL && i<NUM_CHILDS; i++ ) {
        for (int j = 0; j < MAX_TASK_PER_CHILD && argv[currentFile]!=NULL; ++j)
            queueIfFile(argv, communications[i], &currentFile);
    }
    //el primer argumento es el nombre del programa

    for (int filesProcessed=0; argv[currentFile] != NULL && filesProcessed < SHM_SIZE; )   {
        fd_set set = createSet(communications);

        if(select(NUM_CHILDS, &set, NULL, NULL,NULL)!= ERROR)
            errExit("Failed when using select");
        //Check which fd is ready
        for (int i = 0; i < NUM_CHILDS; i++) {
            if(FD_ISSET(communications[i].slaveToMasterFd[READPOS], &set)){
                if (read(communications[i].slaveToMasterFd[READPOS], &(buff[filesProcessed]), DATA_SIZE) == ERROR)
                    errExit("Error al leer del pipe slaveToMaster");
                filesProcessed++;
                if(sem_post(semaphore)==ERROR)
                    errExit("could not execute sem_post");
                queueIfFile(argv, communications[i], &currentFile);
            }
        }
    }
}



