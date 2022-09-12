// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "lib.h"
#include "shmADT.h"




#define NUM_CHILDS 4
#define MEMINC 5
#define DEFAULTTASKNUMBER 1

#define SLAVE_NAME "./slave"
#define RESULTS_NAME "results.csv"
#define SHM_NAME "/appshm"
#define SEM_NAME "/sem"
#define TASKCAPPED 5

typedef struct slaveComm{
    int masterToSlaveFd[PIPESIZE];
    int slaveToMasterFd[PIPESIZE];
    FILE * readStream;
}slaveComm;

//devuelve como valor de retorno el maximo fileDescriptor
int createReadSet(slaveComm * comms, fd_set * set){
    FD_ZERO(set);
    int max;
    for (int i = 0; i < NUM_CHILDS; ++i){
        FD_SET(comms[i].slaveToMasterFd[READPOS], set);
        if ( i == 0 || comms[i].slaveToMasterFd[READPOS] > max)
            max = comms[i].slaveToMasterFd[READPOS];
    }
    return max;
}

void sendDataToFile(FILE * fptr, char * buffer){
    fprintf(fptr,"%s",buffer);
}

void prepareHeaders(FILE * fptr){
    fprintf(fptr,"Hash,FileName,slavePid\n");
}

int isReg(const char* fileName){
    struct stat path;
    if (stat(fileName, &path) == ERROR)
        return false;
    return S_ISREG(path.st_mode);
}

void sendTask(slaveComm *comm, char * file, shmADT data){
    if(write(comm->masterToSlaveFd[WRITEPOS], file, strlen(file)) == ERROR)
        errExitUnlink("Failed at writing on child pipe from master", data);
}

void sendTaskToChild(char * fileName, slaveComm * comm, shmADT data){
    char aux[TRANSFERSIZE];
    sprintf(aux,"%s\n",fileName);
    sendTask(comm,aux, data);
}

void getData(char * buffer, FILE * fptr, shmADT data){
    if(fgets(buffer,TRANSFERSIZE,fptr)==NULL)
        errExitUnlink("Error when reading from slave", data);
}

char ** removeNoReg(char ** argv, int * size, shmADT data){
    char ** regArgv = NULL;
    char ** temp;
    int j = 0;
    for (int i = 1; argv[i] != NULL; i++){
        if (j % MEMINC == 0){
            temp = realloc(regArgv, (MEMINC + j) * sizeof(char *));
            if (temp == NULL) {
                if(regArgv!=NULL)
                    free(regArgv);
                errExitUnlink("failed realloc", data);
            }
            regArgv = temp;
        }
        if (isReg(argv[i]))
            regArgv[j++] = argv[i];
    }
    temp = realloc(regArgv,(j + 1) * sizeof(char *));
    if (temp == NULL) {
        free(regArgv);
        errExitUnlink("Failed realloc", data);
    }
    regArgv = temp;
    regArgv[j] = NULL;
    *size = j;
    return regArgv;
}

int getMin(int num1,int num2){
    return num1 < num2 ? num1:num2;
}

int getNumberOfFilesPerChild(int fileNum){
    int result = (int) (fileNum * 0.10 / NUM_CHILDS) ;
    return result == 0 ? DEFAULTTASKNUMBER:getMin(TASKCAPPED,result);
}

void createSlaves(slaveComm * communications,shmADT shareData){

    for (int i = 0; i < NUM_CHILDS ; i++){
        if (pipe(communications[i].masterToSlaveFd) == ERROR)
            errExitUnlink("Error in master to slave pipe creation", shareData);
        if ( pipe(communications[i].slaveToMasterFd) == ERROR)
            errExitUnlink("Error in slave to master pipe creation", shareData);

        pid_t  myPid =  fork();
        if(myPid == ERROR)
            errExitUnlink("Fork could not be executed", shareData);
        if (myPid == 0){
            close(communications[i].masterToSlaveFd[WRITEPOS]);
            close(communications[i].slaveToMasterFd[READPOS]);

            dup2(communications[i].slaveToMasterFd[WRITEPOS],STDOUT_FILENO);
            close(communications[i].slaveToMasterFd[WRITEPOS]);

            dup2(communications[i].masterToSlaveFd[READPOS],STDIN_FILENO);
            close(communications[i].masterToSlaveFd[READPOS]);

            char * args[] = {NULL};
            int retVal = execv(SLAVE_NAME,args);
            if (retVal == ERROR)
                errExitUnlink("Error in execv syscall", shareData);
        }
        close(communications[i].slaveToMasterFd[WRITEPOS]);
        close(communications[i].masterToSlaveFd[READPOS]);
    }
}

void createFileStream(slaveComm * communications,shmADT shareData){
    for ( int i = 0; i < NUM_CHILDS ; i++){
        FILE * fdptr = fdopen(communications[i].slaveToMasterFd[READPOS],"r");
        if ( fdptr == NULL)
            errExitUnlink("Error creating fileStream",shareData);
        communications[i].readStream =  fdptr;
    }
}

void closeFileStream(slaveComm * comms){
    for ( int i = 0; i < NUM_CHILDS ; i++){
        fclose(comms[i].readStream);
    }
}

void processFiles(char ** argv,slaveComm * communications,FILE * resultFile,shmADT shareData){
    int cantRegFiles;
    char ** regArgV = removeNoReg(argv,&cantRegFiles, shareData);

    //calculamos la cantidad de archivos a procesar por hijo, en base a los archivos regulares que nos pasaron (ver criterio en informe)
    int filesPerChild = getNumberOfFilesPerChild(cantRegFiles);
    int currentFile = 0;

    for (int i = 0; i < NUM_CHILDS && regArgV[currentFile] != NULL ; i++)
        for (int j = 0; j < filesPerChild && regArgV[currentFile] != NULL ;j++)
                sendTaskToChild(regArgV[currentFile++],&communications[i], shareData);

    for (int filesProccesed = 0; filesProccesed < cantRegFiles;){
        fd_set readSet;
        int maxfd = createReadSet(communications,&readSet);
        if (select(maxfd + 1, &readSet, NULL,NULL,NULL) == ERROR)
            errExitUnlink("Error while using select", shareData);

        for ( int i =0 ; i < NUM_CHILDS ; i++){
            if (FD_ISSET(communications[i].slaveToMasterFd[READPOS],&readSet)){
                char buffer[REGBUFFSIZE];
                getData(buffer,communications[i].readStream, shareData);

                if(shmWriter(shareData,buffer)==ERROR)
                    errExitUnlink("Error when writing to shm", shareData);

                sem_post(getSem(shareData));
                sendDataToFile(resultFile,buffer);
                filesProccesed++;

                if ( regArgV[currentFile] != NULL) {
                    sendTaskToChild(regArgV[currentFile],&communications[i], shareData);
                    currentFile++;
                }
            }
        }
    }
    free(regArgV);
    sem_post(getSem(shareData));
}

int main(int argc, char *argv[]){

    if(argc<2)
        errExit("Application process did not recieve enough arguments");

    struct stat s;
    fstat(STDOUT_FILENO,&s);
    if (S_ISFIFO(s.st_mode))
        printf("%s,%s\n", SHM_NAME, SEM_NAME);
    else
        printf("Los parametros que tiene que ingresar en el proceso view son: %s,%s\n", SHM_NAME, SEM_NAME);

    shmADT shareData = initiateSharedData(SHM_NAME, SEM_NAME, SHM_SIZE);
    if ( shareData == NULL)
        errExit("Could not allocate data for process view");
    
    if(shareData==NULL)
        errExit("Error when initiating shared data");
    fflush(stdout);
    sleep(2);
    
    slaveComm communications[NUM_CHILDS];
    createSlaves(communications,shareData);
    createFileStream(communications,shareData);

    FILE * resultFile = fopen(RESULTS_NAME,"w+");
    if(resultFile == NULL)
        errExitUnlink("Cannot create results.csv file", shareData);

    prepareHeaders(resultFile);
    processFiles(argv,communications,resultFile,shareData);
    fclose(resultFile);
    closeFileStream(communications);
    if(closeShm(shareData)==ERROR)
        errExit("Error when closing/unlinking shm");

    return 0;
}
