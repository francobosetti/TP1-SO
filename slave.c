// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "lib.h"
#include "shmADT.h"
#define TRANSFERSIZE 64


//master --> "nombreArchivo\n"

//slave --> "hash,nombreArchivo,slavePid\n"
void prepareData(char *buffer, char * fileName){
    char aux[TRANSFERSIZE];
    //copiamos el hash
    int i;
    for ( i = 0; buffer[i] != ' '; i++){
        aux[i] = buffer[i];
    }
    aux[i] = 0;

    //reemplazamos la newline recibida con fgets en el nombre de archivo
    sprintf(buffer,"%s,%s,%d\n",aux, fileName, getpid());

}

void replaceNewLine(char * str){
    int foundNewLine = false;
    for (int i = 0; !foundNewLine && str[i] != 0 ; i++){
        if (str[i] == '\n'){
            foundNewLine = true;
            str[i] = 0;
        }
    }
}

int main(void){
    char entry[TRANSFERSIZE];
    char buffer[MAX_BUFF];
    while ( fgets(entry,TRANSFERSIZE,stdin) != NULL ){
        //el buffer termina con \n
        replaceNewLine(entry);
        sprintf(buffer,"md5sum %s",entry);
        FILE * md5Ptr = popen(buffer,"r");
        if (md5Ptr == NULL)
            errExit("Error calculating hash");
        fgets(buffer,MAX_BUFF,md5Ptr);
        pclose(md5Ptr);
        prepareData(buffer, entry);
        write(STDOUT_FILENO,buffer, strlen(buffer));
    }

    return 0;
}