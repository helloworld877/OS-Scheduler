#include "headers.h"




int pid;
int main(int agrc, char *argv[])
{
    char buff1[5]; //4 for int + 1 for null terminator     
    int process = 20;
    sprintf(buff1, "%d", process);
    pid = fork();
    argv[1] = buff1;
    if ( pid == 0 )
        execv("./process.out", argv);
    
}