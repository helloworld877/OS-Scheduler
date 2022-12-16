#include "headers.h"

#define MAX 50

struct message
{
     long m_type;
     int message_data[4];
};

int main(int argc, char *argv[])
{
    //TODO: implement the scheduler.
    //TODO: upon termination release the clock resources.
    initClk();

    //Variables received through execv() of process_generator:
    int processes_number = atoi(argv[1]);
    int algo_number = atoi(argv[2]);
    int quantum = atoi(argv[3]);

    //Message variables:
    key_t key_sch_pgen=33; //key associated with the message queue
    //use IPC_CREAT to create the message queue if it does not exist
    int msgq_id = msgget(key_sch_pgen, 0666 | IPC_CREAT);  // creating message queue
    struct message msg;
    int received; //to store return of msgrcv 

    if(msgq_id==-1)
    {
        perror("Error in creation");
        exit(-1);
    }

    //Structs and general variables:
    Queue *readyQueue=createQueue();
    Node *Node_to_insert;
    Node *p_executing=NULL; //process that will be executing
    int received_number=0; //number of processes received so far
    int time=getClk();
    int nexttime;
    int *p_PIDS = (int *)malloc(processes_number * sizeof(int)); //For process PIDs
    int indexPID = 0;

//Algorithm execution
switch(algo_number)
{
    case 1: //Shortest Job First

    while(!isEmpty(readyQueue) || (received_number<processes_number))
    {
        //We will break out of the below loop when we do not receive any message and our readyQueue is not empty
        do
        {
            //Do not wait for a message
            received=msgrcv(msgq_id,&msg,sizeof(msg.message_data), 0, IPC_NOWAIT);

            //msgrcv returns -1 if no message was received
            if(isEmpty(readyQueue) && received==-1) //no processes present to perform
            {
                printf("Ready queue is empty. Waiting to receive a new process...\n");
                printf("Current time : %d \n",getClk());
                printf("Total processes received till now : %d\n", received_number);
                printf("Remaining processes that have still not arrived: %d",processes_number-received_number);
                printf("\n");
                //wait for a message
                received=msgrcv(msgq_id,&msg,sizeof(msg.message_data), 0, !IPC_NOWAIT);
            }

            //if a message has been received
            if(received != -1) 
            {
                // printf("Process with ID %d has just arrived\n",msg.message_data[0]);
                Node_to_insert=newNode(msg.message_data[0],msg.message_data[1],msg.message_data[2],msg.message_data[3], WAITING);       
                enQueueSJF(readyQueue,Node_to_insert); //create fn to enqueue a node with these info FIFO
                received_number++;
            }
        }while(received != -1); //since different processes can have the same arrival time, if I received a message enter to check if I will receive another one as well

        int pid;
        if (!isEmpty(readyQueue))
        {
            int clockk = getClk();
            int current_child_pid;
            p_executing=peek_queue(readyQueue);
            int burst_time = p_executing->Runtime;
            char buff1[5];
            sprintf(buff1, "%d", burst_time);
            argv[1] = buff1;
            deQueue(readyQueue);
            pid = fork();
            if (pid!=0)
                current_child_pid=pid;
            if (pid == 0) 
                execv("./process.out", argv);
            else
            {
                kill(current_child_pid, SIGUSR2);
                sleep(burst_time);
                printf("Process with ID = %d has finished", p_executing->ID );
                printf("\n");
            }

        }
    
    }
    break;
}



            destroyClk(true);
}
