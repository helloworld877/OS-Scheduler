#include "headers.h"

/*
HOW TO USE PROCESS
->when u create a process send to its pid SIGUSR1 (PAUSE TILL I TELL YOU)
->when it is the turn of the process to run send to its pid SIGUSR2 (CONTINUE YOUR PROCESS)
->if you want to pause te running process send SIGUSR1
kill(pid_of_the _process,SIGUSR1/2)


/

/ Modify this file as needed*/

int remainingtime;




int main(int agrc, char *argv[])
{   
    initClk();
    
    remainingtime = atoi(argv[1]);

    int curr_time = getClk();
    while (remainingtime > 0)
    {
        while(getClk()!= curr_time+1)       
        {}
        remainingtime -= 1;
        // assuming that the clock is 1 second
        curr_time = getClk();
    }

    destroyClk(false);
    
    return 0;
}

