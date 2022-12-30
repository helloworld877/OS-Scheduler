#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

enum STATUS
{
    RUNNING,
    WAITING,
    STOPPED,
    CONTINUE,
    FINISHED
};

typedef struct Node
{
    int ID;
    int Arrival;
    int Runtime;
    int Priority;
    int PID;
    enum STATUS Status;
    struct Node *next;
    int size;

    // For output file
    int Finish_time;    // Time at which process finished
    int Waiting_time;   // Total waiting time
    int Start_time;     // Time at which a process started executing
    int Stopped_time;   // Time at which process stopped executing
    int Remaining_time; // Remaining time for a process
    int TA;             // Turnaround time
    float WTA;          // Weighted turnaround time

} Node;

// Tree node for buddy system
typedef struct TreeNode
{
    struct TreeNode *parent; // parent node
    struct TreeNode *right;  // right child node
    struct TreeNode *left;   // left child node

    // properties
    int size;
    int actual_size;
    int start_byte;
    int end_byte;
} TreeNode;
// Note: used typedef so no need to write type "struct Node n1" with every Node declaration, instead just write "Node n1"

typedef struct Queue
{
    struct Node *Last;
    struct Node *Head;

} Queue;

// Create new Node
Node *newNode(int id, int arrival, int run, int p, enum STATUS stat)
{
    struct Node *tmp = (Node *)malloc(sizeof(Node));
    tmp->ID = id;
    tmp->Arrival = arrival;
    tmp->Runtime = run;
    tmp->Priority = p;
    tmp->Status = stat;
    tmp->next = NULL;
    return tmp;
}

Queue *createQueue()
{
    struct Queue *q = (Queue *)malloc(sizeof(Queue));
    q->Head = NULL;
    return q;
}

void enQueueRR(Queue *q, Node *newNode)
{

    // Node *tmp = newNode;
    // if (q->Head == NULL)
    //     {
    //         q->Head = tmp;
    //         return;
    //     }

    // Node * trav = q->Head;

    // while (trav->next)
    //     trav = trav->next;

    // trav->next = tmp;
    // return;

    // Create a new node
    struct Node *tmp = newNode;

    // If queue is empty, then new node is front and rear both
    if (q->Head == NULL)
    {
        q->Head = tmp;
        q->Last = q->Head;
        return;
    }

    struct Node *Trav = q->Head;
    while (Trav->next)
        Trav = Trav->next;

    tmp->next = Trav->next;
    Trav->next = tmp;
}

void enQueueHPF(Queue *q, Node *newNode)
{
    // Create a new node
    struct Node *tmp = newNode;

    // If queue is empty, then new node is front and rear both
    if (q->Head == NULL)
    {
        q->Head = tmp;
        return;
    }

    else if (q->Head && q->Head->Priority > tmp->Priority)
    {
        tmp->next = q->Head;
        q->Head = tmp;
        return;
    }

    else
    {
        struct Node *trav1 = q->Head;
        struct Node *trav2 = trav1->next;
        int flag = 0;
        while (trav2 != NULL)
        {
            if (trav2->Priority > tmp->Priority)
            {
                trav1->next = tmp;
                tmp->next = trav2;
                flag = 1;
                break;
            }
            else
            {
                trav1 = trav1->next;
                trav2 = trav2->next;
            }
        }

        if (!flag)
            trav1->next = tmp;
    }
}

void enQueueSJF(Queue *q, Node *newNode)
{
    // Create a new node
    struct Node *tmp = newNode;

    // If queue is empty, then new node is front and rear both
    if (q->Head == NULL)
    {
        q->Head = tmp;
        return;
    }

    else if (q->Head && q->Head->Runtime > tmp->Runtime)
    {
        tmp->next = q->Head;
        q->Head = tmp;
        return;
    }

    else
    {
        struct Node *trav1 = q->Head;
        struct Node *trav2 = trav1->next;
        int flag = 0;
        while (trav2 != NULL)
        {
            if (trav2->Runtime > tmp->Runtime)
            {
                trav1->next = tmp;
                tmp->next = trav2;
                flag = 1;
                break;
            }
            else
            {
                trav1 = trav1->next;
                trav2 = trav2->next;
            }
        }

        if (!flag)
            trav1->next = tmp;
    }
}

void printqueue(Queue *q)
{
    struct Node *Trav = q->Head;
    while (Trav)
    {
        printf("%d ", Trav->ID);
        Trav = Trav->next;
    }
}

bool isEmpty(Queue *q)
{
    if (q->Head)
        return false;

    return true;
}

void deQueue(struct Queue *q)
{
    if (isEmpty(q))
        return;
    q->Head = q->Head->next;
}

Node *peek_queue(Queue *q) // return ptr on head
{
    return q->Head;
}