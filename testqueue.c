#include "headers.h"
#include <stdbool.h>



void main()
{
    Queue * Q1 = createQueue();
    Node* n1 = newNode(103, 3, 5, 0, RUNNING);
    Node* n2 = newNode(102, 3, 5, 0, RUNNING);
    Node* n3 = newNode(101, 3, 5, 0, RUNNING);
    Node* n4 = newNode(100, 3, 5, 0, RUNNING);// change any of these paramters to test the queue functions
    enQueueRR(Q1,n1);
    enQueueRR(Q1,n2);
    enQueueRR(Q1,n3);
    enQueueRR(Q1,n4);
    printqueue(Q1);

}
