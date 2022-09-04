#include "includes.h"

//we have to define number of child that the parent will create and how many tasks can a child make at MAX
// _BSD-SOURCE --> VER BIEN ESTO, ES PARA LA MACRO ISDIR
#define NUM_CHILDS 4
#define MAX_TASK_PER_CHILD 3
#define PIPESIZE 2
#define ERROR -1
#define READPOS 0
#define WRITEPOS 1

#define SLAVE_NAME "./slave"
#define ANSWER_NAME "./Answers.txt"

typedef struct slaveComm{
    int masterToSlaveFd[PIPESIZE];
    int slaveToMasterFd[PIPESIZE];
}slaveComm;

int is_dir(const char* fileName){
    struct stat path;
    stat(fileName, &path);
    return S_ISDIR(path.st_mode);
}

void errExit(char * msg){
    perror(msg);
    exit(ERROR);
}

fd_set create_set(slaveComm *comms){
    fd_set readSet;
    FD_ZERO(&readSet);
    for (int i = 0; i < NUM_CHILDS; ++i)
        FD_SET(comms[i].slaveToMasterFd[READPOS], &readSet);
    return readSet;
}

int main(int argc, char *argv[]){

    if(argc<2){
        errExit("App no recibio cantidad de argumentos suficientes");
    }
    //Creating the shared memory segment
    int fdParent = shm_open(SHM_NAME, O_CREAT|O_RDWR, S_IRUSR|S_IWUSR);
    if(fdParent == -1)
        errExit("shm_open could not be executed");

    if(ftruncate(fdParent, SHM_SIZE))
        errExit("Space could not be allocated to shared memory segment");

    void * shmPointer = mmap(NULL, SHM_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fdParent, 0);
    if(shmPointer == MAP_FAILED)
        errExit("Shared memory segment could not be mapped to virtual memory of application process");

    //Circular vector to be used by multiple processes
    char * buff = (char *) shmPointer;


    //vector de comunicacion para pipes
    slaveComm communications[NUM_CHILDS];
    //inicializacion de procesos esclavos
    for (int i = 0; i < NUM_CHILDS ; i++){
        if (pipe(communications[i].masterToSlaveFd) == ERROR)
            errExit("error in master to slave pipe creation");
        if ( pipe(communications[i].slaveToMasterFd) == ERROR)
            errExit("error in slave to master pipe creation");
        pid_t  myPid =  fork();
        if(myPid < 0)
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

    //el primer argumento es el nombre del programa

    for ( int current = 1; argv[current] != NULL ;)   {

        fd_set set = create_set(communications);

        //Check which fd is ready
        for (int i = 0; i < NUM_CHILDS; ++i) {

            // 1- entro con todo vacio
            // 2- --> ready todos los write, not raedy todos los read
            // 3---> mando datos
            // 4 --> reseteo


            /*
            if(FD_ISSET(communications[i].slaveToMasterFd[READPOS], &set)){
                data comResult;
                if (read(communications[i].slaveToMasterFd[READPOS], &comResult, DATA_SIZE) == ERROR)
                    errExit("Error al leer del pipe slaveToMaster");
            }
            */
        }
    }

}

