#include "headers.h"

/* Modify this file as needed*/
int remainingtime;
int msgq_id;

int SIGUSR1_handler(int signum);
int SIGUSR2_handler(int signum);
int main(int agrc, char *argv[])
{
    signal(SIGUSR1, SIGUSR1_handler);
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
    sigemptyset(&sigact.sa_mask);
    sigact.sa_flags = 0;
    sigact.sa_handler = SIGUSR2_handler;
    sigaction(SIGUSR1, &sigact, NULL);
    pause();
}
int SIGUSR2_handler(int signum)
{
    return;
}
