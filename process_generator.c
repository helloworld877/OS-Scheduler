#include "headers.h"

// global variables:
int msgq_id;
int send_data;
int clock_id;
int scheduler_id;

// void clearResources(int);

struct message
{
    long m_type;
    int message_data[5];
};
void clearResources(int signum);
int SIGTSTPP_handler(int signum);

int main(int argc, char *argv[])
{
    signal(SIGINT, clearResources);
    signal(SIGTSTP, SIGTSTPP_handler);
    // TODO Initialization
    // 1. Read the input files.
    /*
    Expected command if RR:
    ./process_generator.out processes.txt -sch 3 -q 2
    else:
    ./process_generator.out processes.txt -sch 1
    */
    // validating input arguments provided in the terminal
    if (argc < 4)
    {
        printf("Too few arguments...\n");
        exit(1);
    }
    else if (argc > 6)
    {
        printf("Too many arguments...\n");
        exit(1);
    }
    if (argc == 6) // if quantum is provided
    {
        if (atoi(argv[3]) != 3) // and chosen algorithm not RR
        {
            printf("Wrong arguments...\n");
            exit(1);
        }
    }

    // reading inputs from input file
    FILE *fp = fopen("processes.txt", "r");
    int processes_num = 0;
    int character = 0;
    char chr = getc(fp);
    while (chr != EOF)
    {
        // Count whenever new line is encountered
        if (chr == '\n')
            processes_num += 1;

        // take next character from file.
        chr = getc(fp);
    }

    processes_num -= 1;
    // Make 2D array to hold process data
    int process_data[processes_num][5];

    // reset the file ptr to the beginning of the file
    rewind(fp);

    + // skip commented line at that is at the beginning
        fscanf(fp, "%*[^\n]\n");

    // initialize 2D array with processes' data
    for (int i = 0; i < processes_num; i++) // each row
    {
        for (int j = 0; j < 5; j++) // each coloumn
            fscanf(fp, "%d", &process_data[i][j]);
    }

    // done with reading file, close it
    fclose(fp);

    // 2. Read the chosen scheduling algorithm and its parameters, if there are any from the argument list.
    int algo = atoi(argv[3]);
    int quantum = 0;
    if (algo == 3) // if RR is chosen
        quantum = atoi(argv[5]);

    // Inform user with his choice
    printf("You have chosen : ");
    switch (algo)
    {
    case 1:
        printf("Shortest Job First\n");
        break;
    case 2:
        printf("Preemptive Highest Priority First\n");
        break;
    case 3:
        printf("Round Robin with quantum of %d\n", quantum);
        break;
    case 4:
        printf("Multiple level Feedback Loop\n");
        break;
    }

    // 3. Initiate and create the scheduler and clock processes.
    /*
    execv() function replaces the current process image with a new process image specified by path.
    The new image is constructed from a regular, executable file called the new process image file.
    No return is made because the calling process image is replaced by the new process image. Therefore we will use fork before execv()
    */
    // For clock process
    // argv list is unused in clk.c therefore no need to change it
    int pid;
    pid = fork();
    if (pid == 0) // child process i.e. clock process
    {
        clock_id = getpid();
        if (execv("./clk.out", argv) == -1)
            perror("failed to execv for clock process");
    }

    // For scheduler process
    // argv list should change (inside scheduler process), note that this should be initialized with char
    pid = fork();

    if (pid == 0) // child process i.e. scheduler process
    {
        scheduler_id = getpid();
        printf("%d", scheduler_id);
        // To change our data to char, we will use character buffers
        char buff1[5]; // 4 for int + 1 for null terminator
        char buff2[5];
        char buff3[5];

        sprintf(buff1, "%d", processes_num);
        sprintf(buff2, "%d", algo);
        sprintf(buff3, "%d", quantum); // quantum would be 0 if not RR

        argv[1] = buff1;
        argv[2] = buff2;
        argv[3] = buff3;

        if (execv("./scheduler.out", argv) == -1)
            perror("failed to execv for scheduler process");
    }
    scheduler_id = pid;

    // 4. Use this function after creating the clock process to initialize clock.
    initClk();
    // To get time use this function.
    int x = getClk();
    // printf("Current Time is %d\n", x);

    // TODO Generation Main Loop
    // 5. Create a data structure for processes and provide it with its parameters.
    key_t key_sch_pgen = 33; // key associated with the message queue
    // use IPC_CREAT to create the message queue if it does not exist
    msgq_id = msgget(key_sch_pgen, 0666 | IPC_CREAT); // creating message queue
    struct message msg;

    if (msgq_id == -1)
    {
        perror("Error in creation");
        exit(-1);
    }

    // 6. Send the information to the scheduler at the appropriate time.
    for (int current_id = 0; current_id < processes_num; current_id++)
    {
        // Get time
        // int time = getClk();
        while (getClk() < process_data[current_id][1]) // The process whose turn it is has not yet arrived
        {
            sleep(process_data[current_id][1] - getClk()); // sleep until it arrives
        }

        if (getClk() == process_data[current_id][1]) // if a process has arrived
        {
            msg.m_type = process_data[current_id][0];
            msg.message_data[0] = process_data[current_id][0]; // process id
            msg.message_data[1] = process_data[current_id][1]; // arrival time
            msg.message_data[2] = process_data[current_id][2]; // running time
            msg.message_data[3] = process_data[current_id][3]; // priority
            msg.message_data[4] = process_data[current_id][4]; // mem_size
            send_data = msgsnd(msgq_id, &msg, sizeof(msg.message_data), !IPC_NOWAIT);
        }
    }

    // 7. Clear clock resources

    int status;
    int test = waitpid(scheduler_id, &status, 0);
    printf("exit status %d\n", WEXITSTATUS(status));
    if (WIFEXITED(status))
    {
        int msgq_del;
        msgq_del = msgctl(msgq_id, IPC_RMID, 0);
        destroyClk(true);
        exit(0);
    }
}

void clearResources(int signum)
{
    int msgq_del;
    printf("Caught Signal SIGINT, Clearing All resources\n");
    msgq_del = msgctl(msgq_id, IPC_RMID, 0);
    destroyClk(true);
    exit(0);
}

int SIGTSTPP_handler(int signum)
{
    return 0;
}
