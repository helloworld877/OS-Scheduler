#include <stdio.h>
#include <stdlib.h>

enum STATUS
{
    RUNNING,WAITING,STOPPED,CONTINUE,FINSIHED
};

typedef struct Node
{
    int ID;
    int Arrival;
    int Runtime;
    int Priority;
    enum STATUS Status;
    Node *next;

    //For output file
    int Finish_time;
    int Waiting_time;
    int Start_time;
    int Stopped_time;
    int Running_time;
    int Remaining_time;

}Node;

//Note: used typedef so no need to write type "struct Node n1" with every Node declaration, instead just write "Node n1"

typedef struct Queue
{
    Node *Head;
}Queue;


//Create new Node
Node *newNode(int id, int arrival, int run, int p,enum STATUS stat)
{
    Node *tmp = (Node *)malloc(sizeof(Node));
    tmp->ID = id;
    tmp->Arrival = arrival;
    tmp->Runtime = run;
    tmp->Priority = p;
    tmp->Status=stat;
    tmp->next=NULL;
    return tmp;
}

Queue* createQueue()
{
    Queue *q=(Queue*)malloc(sizeof(Queue));
    q->Head=NULL;
    return q;
}

void enQueueRR(Queue *q,  Node* newNode)
{
    // Create a new node
    Node *tmp = newNode;

    // If queue is empty, then new node is front and rear both
    if (q->Head == NULL)
    {
        q->Head = tmp;
        return;
    }

    //TO-DO: Else insert at the end of the queue
}

void enQueueHPF(Queue *q,  Node* newNode)
{
    // Create a new node
    Node *tmp = newNode;

    // If queue is empty, then new node is front and rear both
    if (q->Head == NULL)
    {
        q->Head = tmp;
        return;
    }

    Node *parent = q->Head;
    if (parent->Priority > tmp->Priority)
    {
        // Insert New Node before head
        tmp->next = q->Head;
        q->Head = tmp;
    }
    else
    {
        // TO-DO: Traverse the list and find a position to insert the new node

    }
}

void enQueueSJF(Queue *q,  Node* newNode)
{
    // Create a new node
    Node *tmp = newNode;

    // If queue is empty, then new node is front and rear both
    if (q->Head == NULL)
    {
        q->Head = tmp;
        return;
    }

    Node *parent = q->Head;
    if (parent->Runtime > tmp->Runtime)
    {
        // Insert New Node before head
        tmp->next = q->Head;
        q->Head = tmp;
    }
    else
    {
        // TO-DO raverse the list and find a position to insert the new node

    }
}

void deQueue(struct Queue *q)
{
    //TO-DO
}

void printqueue(Queue *q)
{
    //TO-DO
}

bool isEmpty(Queue *q)
{
    //TO-DO
}

Node* peek_queue(Queue* q) //return ptr on head
{
    //TO-DO
}