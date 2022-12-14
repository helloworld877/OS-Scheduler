#include "headers.h"
#include <string.h>
#include <sys/ipc.h>
#include <sys/types.h>
//for msgsnd()
#include <sys/msg.h>
//for msgget()
#include <sys/stat.h>
#include <limits.h>
#define MAX 50

int main(int argc, char *argv[])
{
    initClk();

 //TODO: implement the scheduler.
 //TODO: upon termination release the clock resources.
 //Useful general fns to help us:
//1. get minimum from queue SJF, HPF

//argv[2]: chosen algorithm 
//switch case 3ala 7asab anhy algorithm

int processes_queue[MAX];
int processes_Burst_Time[MAX];
int processes_PRIO[MAX];
int IND = 0;


bool isEmpty()
{
    return IND == 0;
}
void enqueue(int PID, int PRIO, int burst_time)
{
    
    processes_queue[IND] = PID;
    processes_PRIO[IND] = PRIO;
    processes_Burst_Time[IND] = burst_time;
    IND++;  
}

int search_min_burst_time() // returns the position of the least burst-time in the queue, it's non preemptive thus you won't use
// this function as you will be dequeuing the minimum from the queue everytime :D 
{
    if (!isEmpty())
    {
        int min_burst_time = INT_MAX;
        int pos;
        for (int i = 0; i<IND; i++)
        {
            if (processes_Burst_Time[i]<min_burst_time)
            {
                min_burst_time = processes_Burst_Time[i];
                pos=i;
            }
        }
        return pos;
    }
}


int search_max_priority()  //Same as peek() or front(); returns for you the pos of the max priority (used processes_PRIO[i] to compare its
// priority with the current one your are running in case of HPF)
{
    if (!isEmpty())
    {
        int Min_PRIO = INT_MAX;
        int pos;
        for (int i = 0; i<IND; i++)
        {
            if (processes_PRIO[i]<Min_PRIO)
            {
                Min_PRIO = processes_PRIO[i];
                pos=i;
            }
        }
        return pos;

    }
}

int dequeue_priority()
{
    int pos = search_max_priority();
    int val = processes_queue[pos];
    if (!isEmpty())
    {
        for (int i = pos; i<IND; i++)
        {
            processes_queue[i]=processes_queue[i+1];
            processes_PRIO[i] = processes_PRIO[i+1];
            processes_Burst_Time[i] = processes_Burst_Time[i+1];

        }
        IND--;
        return val;
    }

}
int dequeue_burst_time() // returns the PID of the process which has the the minimum Burst Time AND REMOVES IT FROM THE QUEUE!
{   
    
    int pos = search_min_burst_time();
    int val = processes_queue[pos];
    if (!isEmpty())
    {
        for (int i = pos; i<IND; i++)
        {
            processes_queue[i]=processes_queue[i+1];
            processes_PRIO[i] = processes_PRIO[i+1];
            processes_Burst_Time[i] = processes_Burst_Time[i+1];

        }
        IND--;
        return val;
    }

}




destroyClk(true);
}
