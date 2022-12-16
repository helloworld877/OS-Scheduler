#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

enum STATUS
{
    RUNNING,WAITING,STOPPED,CONTINUE,FINISHED
};

typedef struct Node
{
    int ID;
    int Arrival;
    int Runtime;
    int Priority;
    enum STATUS Status;
    struct Node* next;

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
    struct Node *Head;
}Queue;


//Create new Node
Node *newNode(int id, int arrival, int run, int p,enum STATUS stat)
{
    struct Node *tmp = (Node *)malloc(sizeof(Node));
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
    struct Queue *q=(Queue*)malloc(sizeof(Queue));
    q->Head=NULL;
    return q;
}

void enQueueRR(Queue *q,  Node* newNode)
{
    // Create a new node
    struct Node *tmp = newNode;

    // If queue is empty, then new node is front and rear both
    if (q->Head == NULL)
    {
        q->Head = tmp;
        return;
    }

    {
        struct Node * Trav = q->Head;
        while (Trav->next)
            Trav = Trav->next;

        Trav->next = tmp;
    }
}

void enQueueHPF(Queue *q,  Node* newNode)
{
    // Create a new node
    struct Node* tmp = newNode;
  
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
        struct Node*trav1 = q->Head;
        struct Node*trav2 = trav1->next;
        int flag = 0;
        while (trav2!=NULL)
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

void enQueueSJF(Queue *q,  Node* newNode)
{
     // Create a new node
    struct Node* tmp = newNode;
  
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
        struct Node*trav1 = q->Head;
        struct Node*trav2 = trav1->next;
        int flag = 0;
        while (trav2!=NULL)
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
    struct Node* Trav = q->Head;
    while(Trav)
    {
        printf("%d ",Trav->ID);
        Trav=Trav->next;
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

Node* peek_queue(Queue* q) //return ptr on head
{
    return q->Head;
}