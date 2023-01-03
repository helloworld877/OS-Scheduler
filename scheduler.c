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

    // TreeNode *Root = (TreeNode *)malloc(sizeof(TreeNode));
    TreeNode *Root = (TreeNode *)malloc(sizeof(TreeNode));
    Root->left = NULL;
    Root->right = NULL;
    Root->full = 0;
    Root->size = 1024;
    Root->start_byte = 0;
    Root->end_byte = 1023;
    bool res = false;
    bool first_time = true;

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
    FILE *fptr3;
    fptr3 = fopen("memory.log", "w");
    fprintf(fptr3, "#At time x process y state arr w total z remain y wait k \n");

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
    create_file();

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
                    printf("Ready queue is empty. Waiting to receive a new process...\n");
                    printf("Current time : %d \n", getClk());
                    printf("Total processes received till now : %d\n", received_number);
                    printf("Remaining processes that have still not arrived: %d", processes_number - received_number);
                    printf("\n");
                    // wait for a message
                    received = msgrcv(msgq_id, &msg, sizeof(msg.message_data), 0, !IPC_NOWAIT);
                    printf("sent!");
                }

                // if a message has been received
                if (received != -1)
                {

                    Node_to_insert = newNode(msg.message_data[0], msg.message_data[1], msg.message_data[2], msg.message_data[3], msg.message_data[4], WAITING);
                    Tree_Insert(Root, Node_to_insert);
                    fprintf(fptr3, "At time  %d allocated %d bytes for process %d from %d to %d\n", getClk(),
                            Node_to_insert->size,
                            Node_to_insert->ID,
                            Node_to_insert->tree_position->start_byte,
                            Node_to_insert->tree_position->end_byte);
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
                char buff2[5];
                sprintf(buff2, "%d", p_executing->size);
                argv[1] = buff1;
                argv[2] = buff2;
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
                    Tree_Delete(Root, p_executing);
                    fprintf(fptr3, "At time  %d freed %d bytes from process %d from %d to %d\n", getClk(),
                            p_executing->size,
                            p_executing->ID,
                            p_executing->tree_position->start_byte,
                            p_executing->tree_position->end_byte);
                }
            }
        }
        break;
        // MARK
    case 2: // Preemptive Highest Priority First

        while (!isEmpty(readyQueue) || (received_number < processes_number))
        {
            // We will break out of the below loop when we do not receive any message and our readyQueue is not empty
            do
            {
                // Do not wait for a message
                received = msgrcv(msgq_id, &msg, sizeof(msg.message_data), 0, IPC_NOWAIT);
                // printf("message timer: %d\n", getClk());
                // msgrcv returns -1 if no message was received
                if (isEmpty(readyQueue) && received == -1) // no processes present to perform
                {
                    received = msgrcv(msgq_id, &msg, sizeof(msg.message_data), 0, !IPC_NOWAIT);
                }

                if (received != -1)
                {
                    printf("%d ", processes_number);
                    printf("Process with ID %d has just arrived at time %d\n", msg.message_data[0], getClk());
                    Node_to_insert = newNode(msg.message_data[0], msg.message_data[1], msg.message_data[2], msg.message_data[3], msg.message_data[4], WAITING);
                    Node_to_insert->Remaining_time = Node_to_insert->Runtime;
                    Node_to_insert->Waiting_time = 0;
                    Node_to_insert->Stopped_time = getClk();
                    Tree_Insert(Root, Node_to_insert);
                    fprintf(fptr3, "At time  %d allocated %d bytes for process %d from %d to %d\n", getClk(),
                            Node_to_insert->size,
                            Node_to_insert->ID,
                            Node_to_insert->tree_position->start_byte,
                            Node_to_insert->tree_position->end_byte);
                    enQueueHPF(readyQueue, Node_to_insert); // create fn to enqueue a node with these info FIFO
                    received_number++;
                    res = true;
                }
            } while (received != -1); // since different processes can have the same arrival time, if I received a message enter to check if I will receive another one as well

            nexttime = getClk();

            // This below block is called every 1 second:
            if (nexttime > time)
            {
                printf("entered at time: %d\n", nexttime);
                printqueue(readyQueue);
                printf("\n");

                time = nexttime;
                res = false;
                if (!first_time)
                {
                    kill(p_executing->PID, SIGTSTP); // pause process

                    // process had changed
                    if (p_executing->ID != peek_queue(readyQueue)->ID)
                    {
                        printf("//////////////////\n");
                        if (p_executing->Remaining_time != 0)
                        {

                            printf("stopped process: %d\n", p_executing->ID);
                            fprintf(fptr, "At time  %d  process %d paused arr %d total %d remain %d wait %d \n", getClk(),
                                    p_executing->ID, p_executing->Arrival,
                                    p_executing->Runtime, p_executing->Remaining_time,
                                    p_executing->Waiting_time);
                            p_executing->Status = STOPPED;
                            p_executing->Stopped_time = getClk();
                            // printf("stopped time: %d\n", p_executing->Stopped_time);
                        }

                        if (peek_queue(readyQueue)->Status != WAITING)
                        {

                            printf("resumed process: %d\n", peek_queue(readyQueue)->ID);
                            // printf("stopped time: %d\n", peek_queue(readyQueue)->Stopped_time);

                            peek_queue(readyQueue)->Waiting_time += getClk() - peek_queue(readyQueue)->Stopped_time;
                            // change status to stopped and make process wait for execution
                            fprintf(fptr, "At time  %d  process %d resumed arr %d total %d remain %d wait %d \n", getClk(),
                                    peek_queue(readyQueue)->ID, peek_queue(readyQueue)->Arrival,
                                    peek_queue(readyQueue)->Runtime, peek_queue(readyQueue)->Remaining_time,
                                    peek_queue(readyQueue)->Waiting_time);
                        }
                        printf("//////////////////\n");
                    }
                }
                else
                {
                    first_time = false;
                }

                p_executing = peek_queue(readyQueue);
                // printqueue(readyQueue);
                printf("\n");
                // First time for process to run:
                if (p_executing->Status == WAITING)
                {
                    fflush(stdout);
                    pid_t PID = fork();
                    // printf("////////////PID is %d///////////\n", PID);

                    fflush(stdout);

                    p_executing->Start_time = getClk();

                    if (PID == 0)
                    {
                        // sending parameters to child process
                        char buff1[5];
                        sprintf(buff1, "%d", p_executing->Remaining_time);
                        char buff2[5];
                        sprintf(buff2, "%d", p_executing->size);
                        argv[1] = buff1;
                        argv[2] = buff2;

                        // printf("\nI AM THE CHILLLD\n");

                        if (execv("./process.out", argv) == -1)
                        {
                            perror("Failed to execv\n");
                        }
                        exit(0);
                    }
                    else
                    {
                        // TODO: calculate Waiting_time. Put info in output file
                        // printf("created process with PID=%d for process ID=%d\n", PID, p_executing->ID);
                        p_PIDS[indexPID] = PID;
                        indexPID++;
                        p_executing->Start_time = getClk();
                        p_executing->PID = PID;
                        printf("stopped time: %d\n", p_executing->Stopped_time);
                        p_executing->Waiting_time += getClk() - p_executing->Stopped_time;
                        // change status to stopped and make process wait for execution
                        fprintf(fptr, "At time  %d  process %d started arr %d total %d remain %d wait %d \n", getClk(),
                                p_executing->ID, p_executing->Arrival,
                                p_executing->Runtime, p_executing->Remaining_time,
                                p_executing->Waiting_time);
                        p_executing->Status = STOPPED;
                        kill(p_executing->PID, SIGTSTP); // pause process
                        // allocating memory
                    }
                }

                // if the process finished remove it from the queue
                if (p_executing->Remaining_time == 1)
                {
                    int status;

                    kill(p_executing->PID, SIGCONT); // run process
                    p_executing->Status = FINISHED;
                    int run_time = getClk();

                    sleep(1);
                    // waitpid(p_executing->PID, &status, 0);
                    printf("finished process %d at time %d\n ", p_executing->ID, getClk());
                    // setting the finish time of the process
                    p_executing->Finish_time = getClk();
                    // Calculate TA and WTA
                    p_executing->TA = getClk() - p_executing->Arrival;
                    p_executing->WTA = (float)p_executing->TA / p_executing->Runtime;

                    avgwait = (avgwait + (p_executing->Waiting_time));
                    // calculating summation of avg  weighted turn around time
                    avgwta = (avgwta + (p_executing->WTA));
                    // updating total time
                    total_time = getClk();
                    useful_time = p_executing->Runtime + useful_time;
                    fprintf(fptr, "At time  %d  process %d finished arr %d total %d remain %d wait %d TA %d WTA %.2f \n", getClk(), p_executing->ID, p_executing->Arrival, p_executing->Runtime, p_executing->Remaining_time, p_executing->Waiting_time, p_executing->TA, p_executing->WTA);

                    // deallocating memory
                    Tree_Delete(Root, p_executing);
                    fprintf(fptr3, "At time  %d freed %d bytes from process %d from %d to %d\n", getClk(),
                            p_executing->size,
                            p_executing->ID,
                            p_executing->tree_position->start_byte,
                            p_executing->tree_position->end_byte);

                    // make a dummy node to hold the value
                    Node *dummy = p_executing;
                    // dequeue current process
                    deQueue(readyQueue);
                    // place it in the finished queue
                    // change next to null
                    dummy->next = NULL;
                    enQueueRR(finishedQueue, dummy);
                    // p_executing = peek_queue(readyQueue);
                    p_executing->Remaining_time -= 1;
                }
                else
                {

                    kill(p_executing->PID, SIGCONT); // run process
                    p_executing->Status = RUNNING;

                    int run_time = getClk();

                    sleep(1);

                    p_executing->Remaining_time -= 1; // decrement waiting time
                }
                printf("Process in execution is with ID %d \n", p_executing->ID);
                printf("clock: %d\n", getClk());
                printf("remaining time %d\n", p_executing->Remaining_time);
            }
        }

        break;
    case 3: // Round Robin

        // While there are still processes in the ready queue or there are still processes to be recieved
        while (!isEmpty(readyQueue) || (received_number < processes_number))
        {
            // We will break out of the below loop when we do not receive any message and our readyQueue is not empty
            do
            {
                // Do not wait for a message
                received = msgrcv(msgq_id, &msg, sizeof(msg.message_data), 0, IPC_NOWAIT);

                // msgrcv returns -1 if no message was received
                if ((received_number == finishedProcesses) && received == -1) // no processes present to perform
                {
                    printf("Ready queue is empty. Waiting to receive a new process...\n");
                    printf("Current time : %d \n", getClk());
                    printf("Total processes received till now : %d\n", received_number);
                    printf("Remaining processes that have still not arrived : %d \n", processes_number - received_number);
                    // wait for a message
                    received = msgrcv(msgq_id, &msg, sizeof(msg.message_data), 0, !IPC_NOWAIT);
                }

                // if a message has been received
                if (received != -1)
                {
                    printf("message successful at time %d \n", getClk());
                    Node_to_insert = newNode(msg.message_data[0], msg.message_data[1], msg.message_data[2], msg.message_data[3], msg.message_data[4], WAITING);
                    Tree_Insert(Root, Node_to_insert);
                    fprintf(fptr3, "At time  %d allocated %d bytes for process %d from %d to %d\n", getClk(),
                            Node_to_insert->size,
                            Node_to_insert->ID,
                            Node_to_insert->tree_position->start_byte,
                            Node_to_insert->tree_position->end_byte);
                    enQueueRR(readyQueue, Node_to_insert);
                    received_number++;
                    printf("Current ready queue : ");
                    printqueue(readyQueue);
                    printf("\n");
                }
            } while (received != -1); // since different processes can have the same arrival time, if I received a message enter to check if I will receive another one as well

            nexttime = getClk();

            // This below block is called every 1 second:
            if (nexttime > time)
            {
                time = nexttime;
                int PID;

                // If there is a process that is executing and its state is not finished then enqueue it again
                if (p_executing && p_executing->Status != FINISHED)
                {
                    enQueueRR(readyQueue, p_executing);
                    printf("Current ready queue : ");
                    printqueue(readyQueue);
                    printf("\n");
                }

                p_executing = peek_queue(readyQueue);
                deQueue(readyQueue);

                printf("Process in execution is with ID %d \n", p_executing->ID);

                if (p_executing->Status == WAITING)
                {
                    PID = fork();
                    totalRuntime += p_executing->Runtime;

                    if (PID == 0)
                    {
                        char buff1[5];
                        sprintf(buff1, "%d", p_executing->Remaining_time);
                        char buff2[5];
                        sprintf(buff2, "%d", p_executing->size);
                        argv[1] = buff1;
                        argv[2] = buff2;
                        p_executing->Start_time = getClk();

                        if (execv("./process.out", argv) == -1)
                            perror("Failed to execv for process.c");
                    }
                    // First time for process to run:
                    else
                    {
                        kill(PID, SIGSTOP); // pause process
                        p_executing->PID = PID;
                        p_executing->Start_time = getClk();
                        p_executing->Remaining_time = p_executing->Runtime;
                        // p_executing->Status=RUNNING;
                        printf("Starting process with ID %d and PID %d\n", p_executing->ID, PID);
                        printf("Remaining time of %d is %d\n", p_executing->ID, p_executing->Remaining_time);

                        // Write to output file:
                        p_executing->Waiting_time = p_executing->Start_time - p_executing->Arrival;
                        fprintf(fptr, "At time  %d  process %d started arr %d total %d remain %d wait %d \n", getClk(), p_executing->ID, p_executing->Arrival, p_executing->Runtime, p_executing->Remaining_time, p_executing->Waiting_time);
                    }
                }
                // if process was stopped then resume its processing
                else if (p_executing->Status == STOPPED)
                {
                    // change process status
                    p_executing->Status = CONTINUE;
                    printf("Resuming process with ID %d and PID %d\n", p_executing->ID, p_executing->PID);
                    printf("Remaining time of %d is %d\n", p_executing->ID, p_executing->Remaining_time);

                    // Write to output file
                    p_executing->Waiting_time += getClk() - p_executing->Stopped_time;
                    fprintf(fptr, "At time  %d  process %d resumed arr %d total %d remain %d wait %d \n", getClk(), p_executing->ID, p_executing->Arrival, p_executing->Runtime, p_executing->Remaining_time, p_executing->Waiting_time);
                }

                /* Two possibilities:
                1. The remaining time is greater than one quantum. Therefore, run the process for one quantum and stop.
                2. The remaining time is less than one quantum. Therefore, run the process for the remaining time and stop.
                */

                // 1.
                if (p_executing->Remaining_time > quantum)
                {
                    kill(p_executing->PID, SIGCONT); // continue the process

                    sleep(quantum);

                    // decrease remaining time by one qnatum period
                    p_executing->Remaining_time -= quantum;

                    // stop the process
                    printf("Stopping process with ID %d and PID %d...\n", p_executing->ID, p_executing->PID);
                    printf("Remaining time of %d is %d\n", p_executing->ID, p_executing->Remaining_time);
                    kill(p_executing->PID, SIGSTOP);
                    printf("Process with ID %d and PID %d has stopped\n", p_executing->ID, p_executing->PID);

                    // change the process status
                    p_executing->Status = STOPPED;

                    // Record the time at which the process has stopped
                    p_executing->Stopped_time = getClk();

                    // write to file
                    fprintf(fptr, "At time  %d  process %d stopped arr %d total %d remain %d wait %d \n", getClk(), p_executing->ID, p_executing->Arrival, p_executing->Runtime, p_executing->Remaining_time, p_executing->Waiting_time);

                    // Check if this is the only and last process in the ready queue then run it as batch (until it finishes)
                    if (isEmpty(readyQueue) && finishedProcesses == processes_number - 1)
                    {
                        printf("Process with ID %d and PID %d is the last process, executing till the end\n", p_executing->ID, p_executing->PID);

                        // Sleep for the remaining time
                        sleep(p_executing->Remaining_time);

                        // Update remaining time
                        p_executing->Remaining_time = 0;

                        // change the process status
                        p_executing->Status = FINISHED;

                        Tree_Delete(Root, p_executing);
                        fprintf(fptr3, "At time  %d freed %d bytes from process %d from %d to %d\n", getClk(),
                                p_executing->size,
                                p_executing->ID,
                                p_executing->tree_position->start_byte,
                                p_executing->tree_position->end_byte);

                        // Update finish time
                        p_executing->Finish_time = getClk();

                        // Calculate TA and WTA
                        p_executing->TA = getClk() - p_executing->Arrival;
                        p_executing->WTA = (float)p_executing->TA / p_executing->Runtime;

                        // write to file
                        printf("Finished process with ID %d at time %d \n", p_executing->ID, p_executing->Finish_time);
                        finishedProcesses++;

                        useful_time = p_executing->Runtime + useful_time;
                        // calculating summation of avg waiting time
                        avgwait = (avgwait + (p_executing->Waiting_time));
                        // calculating summation of avg  weighted turn around time
                        avgwta = (avgwta + (p_executing->WTA));

                        Node *dummy = p_executing;
                        dummy->next = NULL;
                        enQueueRR(finishedQueue, dummy);
                        p_executing = NULL;
                        printf("Current finished queue : ");
                        printqueue(finishedQueue);
                        printf("\n");
                        total_time = getClk();
                    }
                }

                // 2.
                else if (p_executing->Remaining_time <= quantum && p_executing->Runtime != 0)
                {

                    kill(p_executing->PID, SIGCONT); // continue the process

                    // Sleep for the remaining time
                    sleep(p_executing->Remaining_time);

                    // kill(p_executing->PID, SIGSTOP);

                    // Update remaining time
                    p_executing->Remaining_time = 0;

                    // change the process status
                    p_executing->Status = FINISHED;
                    Tree_Delete(Root, p_executing);
                    fprintf(fptr3, "At time  %d freed %d bytes from process %d from %d to %d\n", getClk(),
                            p_executing->size,
                            p_executing->ID,
                            p_executing->tree_position->start_byte,
                            p_executing->tree_position->end_byte);

                    // Update finish time
                    p_executing->Finish_time = getClk();

                    // Calculate TA and WTA
                    p_executing->TA = getClk() - p_executing->Arrival;
                    p_executing->WTA = (float)p_executing->TA / p_executing->Runtime;

                    // write to file
                    fprintf(fptr, "At time  %d  process %d finished arr %d total %d remain %d wait %d TA %d WTA %.2f \n", getClk(), p_executing->ID, p_executing->Arrival, p_executing->Runtime, p_executing->Remaining_time, p_executing->Waiting_time, p_executing->TA, p_executing->WTA);

                    printf("Finished process with ID %d at time %d \n", p_executing->ID, p_executing->Finish_time);
                    finishedProcesses++;

                    useful_time = p_executing->Runtime + useful_time;
                    // calculating summation of avg waiting time
                    avgwait = (avgwait + (p_executing->Waiting_time));
                    // calculating summation of avg  weighted turn around time
                    avgwta = (avgwta + (p_executing->WTA));

                    Node *dummy = p_executing;
                    dummy->next = NULL;
                    enQueueRR(finishedQueue, dummy);
                    p_executing = NULL;
                    printf("Current finished queue : ");
                    printqueue(finishedQueue);
                    printf("\n");
                    total_time = getClk();
                }
            }
        }
        break;

    case 4: // Multiple level Feedback Loop
        // While there are still processes in the ready queue or there are still processes to be recieved
        while (!isEmpty(readyQueue) || (received_number < processes_number))
        {
            printf("\nstart while loop: %d\n", getClk());
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
                    // printf("Remaining processes that have still not arrived : %d \n", processes_number - received_number);
                    //  wait for a message
                    received = msgrcv(msgq_id, &msg, sizeof(msg.message_data), 0, !IPC_NOWAIT);
                }

                // if a message has been received
                if (received != -1)
                {
                    Node_to_insert = newNode(msg.message_data[0], msg.message_data[1], msg.message_data[2], msg.message_data[3], msg.message_data[4], WAITING);
                    Node_to_insert->Remaining_time = Node_to_insert->Runtime;
                    Node_to_insert->Waiting_time = 0;
                    Node_to_insert->Stopped_time = getClk();
                    Tree_Insert(Root, Node_to_insert);
                    fprintf(fptr3, "At time  %d allocated %d bytes for process %d from %d to %d\n", getClk(),
                            Node_to_insert->size,
                            Node_to_insert->ID,
                            Node_to_insert->tree_position->start_byte,
                            Node_to_insert->tree_position->end_byte);
                    enQueueHPF(readyQueue, Node_to_insert);
                    received_number++;
                    printf("found\n");
                }
            } while (received != -1); // since different processes can have the same arrival time, if I received a message enter to check if I will receive another one as well

            nexttime = getClk();
            printf("\n%d\n", nexttime);
            if (nexttime > time)
            {
                time = nexttime;
                p_executing = peek_queue(readyQueue);
                deQueue(readyQueue); // dequeuing the proccess with the highest priority currently i.e. excuting the highest level existing

                current_level = p_executing->Priority; // updating current level
                // printf("Current level: %d\n", current_level);

                if (p_executing->Status == WAITING)
                {
                    p_executing->Start_time = getClk();
                    int PID = fork();
                    if (PID == 0)
                    {
                        char buff1[5]; // for ID
                        char buff2[5]; // for Runtime
                        sprintf(buff1, "%d", p_executing->ID);
                        sprintf(buff2, "%d", p_executing->Runtime);
                        argv[1] = buff1;
                        argv[2] = buff2;

                        if (execv("./process.out", argv) == -1)
                        {
                            perror("Failed to execv for process.c");
                            exit(-1);
                        }
                        exit(0);
                    }
                    else // First time for process to run:
                    {

                        p_executing->PID = PID;
                        kill(p_executing->PID, SIGUSR1); // pause process
                        // TODO: calculate Waiting_time. Put info in output file
                        original_priorities[p_executing->ID - 1] = p_executing->Priority;
                        p_PIDS[p_executing->ID - 1] = PID;
                        totalRuntime += p_executing->Runtime;
                        // printf("Starting process with ID %d and PID %d\n", p_executing->ID, PID);
                        // Write to output file:
                        p_executing->Waiting_time = p_executing->Start_time - p_executing->Arrival;
                        fprintf(fptr, "At time  %d  process %d started arr %d total %d remain %d wait %d \n", getClk(), p_executing->ID, p_executing->Arrival, p_executing->Runtime, p_executing->Remaining_time, p_executing->Waiting_time);
                    }
                }
                else if (p_executing->Status == STOPPED) // if process was stopped and is continuing
                {
                    // change process status
                    p_executing->Status = CONTINUE;
                    // continue the process
                    kill(p_executing->PID, SIGUSR2);
                    // printf("Resuming process with ID %d and PID %d\n", p_executing->ID, p_PIDS[p_executing->ID - 1]);
                    // Write to output file
                    p_executing->Waiting_time += getClk() - p_executing->Stopped_time;
                    fprintf(fptr, "At time  %d  process %d resumed arr %d total %d remain %d wait %d \n", getClk(), p_executing->ID, p_executing->Arrival, p_executing->Runtime, p_executing->Remaining_time, p_executing->Waiting_time);
                }
                printf("\n%d\n", getClk());
                while (getClk() != time + 1)
                {
                }
                printf("\n%d\n", getClk());

                p_executing->Remaining_time -= 1;
                if (p_executing->Remaining_time == 0) // process is terminated i.e. remaining time =0
                {
                    finished_processes++;
                    // change process status
                    p_executing->Status = FINISHED;
                    p_executing->Finish_time = getClk();
                    // printf("process %d has finished\n", p_executing->ID);
                    //  Calculate TA and WTA
                    p_executing->TA = p_executing->Finish_time - p_executing->Arrival;
                    p_executing->WTA = (float)p_executing->TA / p_executing->Runtime;

                    useful_time = p_executing->Runtime + useful_time;
                    // calculating summation of avg waiting time
                    avgwait = (avgwait + (p_executing->Waiting_time));
                    // calculating summation of avg  weighted turn around time
                    avgwta = (avgwta + (p_executing->WTA));
                    // write to file
                    fprintf(fptr, "At time  %d  process %d finished arr %d total %d remain %d wait %d TA %d WTA %.2f  \n", p_executing->Finish_time, p_executing->ID, p_executing->Arrival, p_executing->Runtime, p_executing->Remaining_time, p_executing->Waiting_time, p_executing->TA, p_executing->WTA);
                    total_time = getClk();
                    Tree_Delete(Root, p_executing);
                    fprintf(fptr3, "At time  %d freed %d bytes from process %d from %d to %d\n", getClk(),
                            p_executing->size,
                            p_executing->ID,
                            p_executing->tree_position->start_byte,
                            p_executing->tree_position->end_byte);
                }
                else
                {
                    // pause the process
                    kill(p_executing->PID, SIGUSR1);
                    p_executing->Stopped_time = getClk();
                    // change process status
                    p_executing->Status = STOPPED;
                    // printf("process %d has stopped\n", p_executing->ID);
                    fprintf(fptr, "At time  %d  process %d stopped arr %d total %d remain %d wait %d \n", getClk(), p_executing->ID, p_executing->Arrival, p_executing->Runtime, p_executing->Remaining_time, p_executing->Waiting_time);

                    if (current_level == 10) // if I am at the lowest level
                    {
                        // return process to its original priority
                        p_executing->Priority = original_priorities[p_executing->ID - 1];
                        p_executing->next = NULL;
                        enQueueHPF(tempQueue, p_executing);
                        if (isEmpty(readyQueue))
                        {
                            while (!isEmpty(tempQueue))
                            {
                                p_executing = peek_queue(tempQueue);
                                deQueue(tempQueue);
                                p_executing->next = NULL;
                                enQueueHPF(readyQueue, p_executing);
                            }
                        }
                    }
                    else
                    { // degrade priority to execute in another level
                        p_executing->Priority = p_executing->Priority + 1;
                        p_executing->next = NULL;
                        enQueueHPF(readyQueue, p_executing);
                    }
                }
                printf("\n%d\n", getClk());
                printf("######################");
            }
        }
        break;
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
