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
int msgq_id;

int SIGUSR1_handler(int signum);
int SIGUSR2_handler(int signum);

int main(int agrc, char *argv[])
{
    signal(SIGUSR1, SIGUSR1_handler);
    // raise(SIGUSR1);
    initClk();
    remainingtime = atoi(argv[1]);

    while (remainingtime > 0)
    {
        remainingtime -= 1;
        // assuming that the clock is 1 second
        sleep(1);
    }

    destroyClk(false);

    return 0;
}
int SIGUSR1_handler(int signum)
{
    printf("received pause\n");
    struct sigaction sigact;
    sigemptyset(&sigact.sa_mask);
    sigact.sa_flags = 0;
    sigact.sa_handler = SIGUSR2_handler;
    sigaction(SIGUSR2, &sigact, NULL);
    pause();
}
int SIGUSR2_handler(int signum)
{
    printf("recieved continue\n");
}
