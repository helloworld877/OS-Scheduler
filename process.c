#include "headers.h"


/* Modify this file as needed*/
int remainingtime;
int msgq_id;

int main(int agrc, char *argv[])
{
    //initClk();
    remainingtime=atoi(argv[1]);
    printf("\n%d\n", remainingtime);

    //TODO The process needs to get the remaining time from somewhere
    //remainingtime = ??;

    

    while (remainingtime > 0)
    {
        
    }

    //destroyClk(false);

    return 0;
}
