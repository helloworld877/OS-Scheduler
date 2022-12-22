#include "headers.h"
#include <stdbool.h>



void main()
{
    Queue * Q1 = createQueue();
    Node* N1=newNode(3,5,6,4, RUNNING);
    Node* N2=newNode(3,5,6,4, RUNNING);
    Node* N3=newNode(3,5,6,4, RUNNING);
    Node* N4=newNode(3,5,6,4, RUNNING);
    Node* N5=newNode(3,5,6,4, RUNNING);
    Node* N6=newNode(3,5,6,4, RUNNING);
    Node* N7=newNode(3,5,6,4, RUNNING);
    

    enQueueRR(Q1,N1);
    enQueueRR(Q1,N2);
    enQueueRR(Q1,N3);
    enQueueRR(Q1,N4);
    enQueueRR(Q1,N5);
    enQueueRR(Q1,N6);
    enQueueRR(Q1,N7);
    printqueue(Q1);


}
