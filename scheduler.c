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

    //Output file:
    FILE *fptr;
    fptr = fopen("schedular.log", "w");
    fprintf(fptr, "#At time x process y state arr w total z remain y wait k \n");

    //Structs and general variables:
    Queue *readyQueue=createQueue();
    Node *Node_to_insert;
    Node *p_executing=NULL; //process that will be executing
    int received_number=0; //number of processes received so far
    int time=getClk();
    int nexttime;
    int *p_PIDS = (int *)malloc(processes_number * sizeof(int)); //For process PIDs
    int indexPID = 0;
    int finishedProcesses=0;

    //For statistics:
    Queue *finishedQueue=createQueue();
    int totalRuntime=0;

//Algorithm execution
switch(algo_number)
{
    case 1: //Shortest Job First

    break;

    case 2: //Preemptive Highest Priority First
    break;

    case 3: //Round Robin

    //While there are still processes in the ready queue or there are still processes to be recieved
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
                printf("Remaining processes that have still not arrived : %d \n",processes_number-received_number);
                //wait for a message
                received=msgrcv(msgq_id,&msg,sizeof(msg.message_data), 0, !IPC_NOWAIT);
            }

            //if a message has been received
            if(received != -1) 
            {
                Node_to_insert=newNode(msg.message_data[0],msg.message_data[1],msg.message_data[2],msg.message_data[3], WAITING);       
                enQueueRR(readyQueue,Node_to_insert);
                received_number++;
                printf("Current ready queue : ");
                printqueue(readyQueue);
                printf("\n");
            }
        }while(received != -1); //since different processes can have the same arrival time, if I received a message enter to check if I will receive another one as well

    nexttime=getClk();

    //This below block is called every 1 second:
    if(nexttime>time)
    {
        time=nexttime;
        int PID;
        
        //If there is a process that is executing and its state is not finished then enqueue it again
        if(p_executing && p_executing->Status != FINISHED)
        {
            enQueueRR(readyQueue,p_executing);
            printf("Current ready queue : ");
            printqueue(readyQueue);
            printf("\n");
        }

        p_executing=peek_queue(readyQueue);
        deQueue(readyQueue);

        printf("Process in execution is with ID %d \n",p_executing->ID);
        
        if(p_executing->Status == WAITING)
        {
            PID=fork();
            totalRuntime+=p_executing->Runtime;

            if(PID==0)
            {
                char buff1[5]; //for ID
                char buff2[5]; //for Runtime
                sprintf(buff1, "%d", p_executing->ID);
                sprintf(buff2, "%d", p_executing->Runtime);
                argv[1]=buff1;
                argv[2]=buff2;
                p_executing->Start_time=getClk();

                if(execv("./process.out",argv)==-1)
                    perror("Failed to execv for process.c");
            }
            //First time for process to run:
            else
            {
                //TODO: calculate Waiting_time. Put info in output file
                p_PIDS[indexPID]=PID;
                indexPID++;
                p_executing->Start_time=getClk();
                printf("Starting process with ID %d and PID %d\n",p_executing->ID,PID); 

                //Write to output file:
                p_executing->Waiting_time = p_executing->Start_time - p_executing->Arrival;
                fprintf(fptr, "At time  %d  process %d started arr %d total %d remain %d wait %d \n", getClk(), p_executing->ID, p_executing->Arrival, p_executing->Runtime, p_executing->Remaining_time,p_executing->Waiting_time);
            
            }        
        }
        //if process was stopped then resume its processing
        else if(p_executing->Status == STOPPED)
        {
            //change process status
            p_executing->Status = CONTINUE;
            printf("Resuming process with ID %d and PID %d\n",p_executing->ID,p_PIDS[p_executing->ID-1]);

            //Write to output file
            p_executing->Waiting_time+=getClk()-p_executing->Stopped_time;
            fprintf(fptr, "At time  %d  process %d started arr %d total %d remain %d wait %d \n", getClk(), p_executing->ID, p_executing->Arrival, p_executing->Runtime, p_executing->Remaining_time,p_executing->Waiting_time);
            
            //continue the process
            kill(p_PIDS[p_executing->ID-1],SIGUSR2);
        }

        /* Two possibilities:
        1. The remaining time is greater than one quantum. Therefore, run the process for one quantum and stop.
        2. The remaining time is less than one quantum. Therefore, run the process for the remaining time and stop.
        */

       // 1.
       if(p_executing->Remaining_time>quantum)
       {
            //Sleep for one quantum period
            sleep(quantum);

            //stop the process
            printf("Stopping process with ID %d and PID %d...\n",p_executing->ID,p_PIDS[p_executing->ID-1]);
            kill(p_PIDS[p_executing->ID-1],SIGUSR1);
            printf("Process with ID %d and PID %d has stopped\n",p_executing->ID,p_PIDS[p_executing->ID-1]);

            //decrease remaining time by one qnatum period
            p_executing->Remaining_time-=quantum;

            //change the process status
            p_executing->Status=STOPPED;

            //Record the time at which the process has stopped
            p_executing->Stopped_time=getClk();

            //write to file
            fprintf(fptr, "At time  %d  process %d started arr %d total %d remain %d wait %d \n", getClk(), p_executing->ID, p_executing->Arrival, p_executing->Runtime, p_executing->Remaining_time,p_executing->Waiting_time);
       
            //Check if this is the only and last process in the ready queue then run it as batch (until it finishes)
            if(isEmpty(readyQueue) && finishedProcesses==processes_number-1)
            {
                printf("Process with ID %d and PID %d is the last process, executing till the end\n",p_executing->ID,p_PIDS[p_executing->ID-1]);

                //Sleep for the remaining time
                sleep(p_executing->Remaining_time);

                //Update remaining time
                p_executing->Remaining_time=0;

                //change the process status
                p_executing->Status=FINISHED;

                //Update finish time
                p_executing->Finish_time=getClk();

                //Calculate TA and WTA
                p_executing->TA = getClk() - p_executing->Arrival;
                p_executing->WTA = (float)p_executing->TA / p_executing->Runtime;
                
                //write to file
                fprintf(fptr, "At time  %d  process %d started arr %d total %d remain %d wait %d \n", getClk(), p_executing->ID, p_executing->Arrival, p_executing->Runtime, p_executing->Remaining_time,p_executing->Waiting_time);

                printf("Finished process with ID %d at time %d \n",p_executing->ID,p_executing->Finish_time);
                finishedProcesses++;
                enQueueRR(finishedQueue,p_executing);
                printf("Current finished queue : ");
                printqueue(finishedQueue);
                printf("\n");
            }
       }

       // 2.
        else if(p_executing->Remaining_time<=quantum && p_executing->Runtime!=0)
        {
            //Sleep for the remaining time
            sleep(p_executing->Remaining_time);

            //Update remaining time
            p_executing->Remaining_time=0;

            //change the process status
            p_executing->Status=FINISHED;

            //Update finish time
            p_executing->Finish_time=getClk();

            //Calculate TA and WTA
            p_executing->TA=getClk()-p_executing->Arrival;
            p_executing->WTA = (float)p_executing->TA / p_executing->Runtime;

            //write to file
            fprintf(fptr, "At time  %d  process %d started arr %d total %d remain %d wait %d \n", getClk(), p_executing->ID, p_executing->Arrival, p_executing->Runtime, p_executing->Remaining_time,p_executing->Waiting_time);


            printf("Finished process with ID %d at time %d \n",p_executing->ID,p_executing->Finish_time);
            finishedProcesses++;
            enQueueRR(finishedQueue,p_executing);
            printf("Current finished queue : ");
            printqueue(finishedQueue);
            printf("\n");
        }

    }
    }
    break;

    case 4: //Multiple level Feedback Loop
    break;

}

destroyClk(true);
}
