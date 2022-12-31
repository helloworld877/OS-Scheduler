#include "headers.h"

#define MAX 50

struct message
{
    long m_type;
    int message_data[5];
};

int main(int argc, char *argv[])
{
    // TODO: implement the scheduler.
    // TODO: upon termination release the clock resources.
    initClk();

    // Variables received through execv() of process_generator:
    int processes_number = atoi(argv[1]);
    int algo_number = atoi(argv[2]);
    int quantum = atoi(argv[3]);

    //TreeNode *Root = (TreeNode *)malloc(sizeof(TreeNode));
    TreeNode *Root;
    Root->left = NULL;
    Root->right = NULL;
    

    // Message variables:
    key_t key_sch_pgen = 33; // key associated with the message queue
    // use IPC_CREAT to create the message queue if it does not exist
    int msgq_id = msgget(key_sch_pgen, 0666 | IPC_CREAT); // creating message queue
    struct message msg;
    int received; // to store return of msgrcv

    if (msgq_id == -1)
    {
        perror("Error in creation");
        exit(-1);
    }

    // Output files:
    FILE *fptr;
    fptr = fopen("schedular.log", "w");
    fprintf(fptr, "#At time x process y state arr w total z remain y wait k \n");

    FILE *fptr2;
    fptr2 = fopen("scheduler.perf", "w");

    // Structs and general variables:
    Queue *readyQueue = createQueue();
    Node *Node_to_insert;
    Node *p_executing = NULL; // process that will be executing
    int received_number = 0;  // number of processes received so far
    int time = getClk();
    int nexttime;
    int *p_PIDS = (int *)malloc(processes_number * sizeof(int)); // For process PIDs
    int indexPID = 0;
    int finishedProcesses = 0;

    // MLFQ vars
    int current_level;
    int stat_loc;
    int finished_processes;
    int *original_priorities = (int *)malloc(processes_number * sizeof(int));
    Queue *tempQueue = createQueue(); // used to in case a new processes is inserted at a higher level and nedd to be excuted first

    // statistics
    int useful_time = 0;
    float avgwta = 0;
    float avgwait = 0;
    int total_time = 0;

    Queue *finishedQueue = createQueue();
    int totalRuntime = 0;

    int index = 0;

    // Algorithm execution
    switch (algo_number)
    {

    case 1: // Shortest Job First
        while (!isEmpty(readyQueue) || (received_number < processes_number))
        {
            // We will break out of the below loop when we do not receive any message and our readyQueue is not empty
            do
            {
                // Do not wait for a message
                received = msgrcv(msgq_id, &msg, sizeof(msg.message_data), 0, IPC_NOWAIT);

                // msgrcv returns -1 if no message was received
                if (isEmpty(readyQueue) && received == -1) // no processes present to perform
                {
                    // printf("Ready queue is empty. Waiting to receive a new process...\n");
                    // printf("Current time : %d \n", getClk());
                    // printf("Total processes received till now : %d\n", received_number);
                    // printf("Remaining processes that have still not arrived: %d", processes_number - received_number);
                    // printf("\n");
                    // wait for a message
                    received = msgrcv(msgq_id, &msg, sizeof(msg.message_data), 0, !IPC_NOWAIT);
                }

                // if a message has been received
                if (received != -1)
                {

                    Node_to_insert = newNode(msg.message_data[0], msg.message_data[1], msg.message_data[2], msg.message_data[3], msg.message_data[4], WAITING);
                    enQueueSJF(readyQueue, Node_to_insert); // create fn to enqueue a node with these info FIFO
                    received_number++;
                }
            } while (received != -1); // since different processes can have the same arrival time, if I received a message enter to check if I will receive another one as well

            int pid;
            if (!isEmpty(readyQueue))
            {
                int clockk = getClk();
                int current_child_pid;
                p_executing = peek_queue(readyQueue);
                int burst_time = p_executing->Runtime;
                p_executing->Remaining_time = burst_time;
                p_executing->Waiting_time = getClk() - p_executing->Arrival;
                char buff1[5];
                sprintf(buff1, "%d", burst_time);
                argv[1] = buff1;
                deQueue(readyQueue);
                pid = fork();
                if (pid != 0)
                    current_child_pid = pid;
                if (pid == 0)
                {
                    if ((execv("./process.out", argv)) == -1)
                        perror("error:");
                }
                else
                {
                    int status;
                    // kill(current_child_pid, SIGSTOP);
                    // kill(current_child_pid, SIGCONT);
                    // printf("Process with ID = %d has started at time %d ", p_executing->ID, getClk());
                    // printf("\n");
                    Tree_Insert(Root, p_executing);
                    fprintf(fptr, "At time  %d  process %d started arr %d total %d remain %d wait %d \n", getClk(),
                            p_executing->ID, p_executing->Arrival,
                            p_executing->Runtime, p_executing->Remaining_time,
                            p_executing->Waiting_time);
                    p_executing->Finish_time = getClk();

                    p_executing->Remaining_time = 0;
                    wait(status);
                    // printf("Process with ID = %d has finished at time %d ", p_executing->ID, getClk());
                    fprintf(fptr, "At time  %d  process %d finished arr %d total %d remain %d wait %d \n", getClk(),
                            p_executing->ID, p_executing->Arrival,
                            p_executing->Runtime, p_executing->Remaining_time,
                            p_executing->Waiting_time);
                    p_executing->next = NULL;
                    enQueueRR(finishedQueue, p_executing);
                }
            }
        }
        break;

        // MARK
    }

    float cpu_utilization = ((float)useful_time * 1.0 / total_time) * 100.0;
    fprintf(fptr2, "CPU utilization= %.2f\n", cpu_utilization);

    avgwta = (float)avgwta / (processes_number);
    fprintf(fptr2, "Avg WTA= %.2f\n", avgwta);

    avgwait = (float)avgwait / (processes_number);
    fprintf(fptr2, "Avg Waiting= %.2f\n", avgwait);

    fclose(fptr);
    fclose(fptr2);

    destroyClk(false);
    return 0;
}
