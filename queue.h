#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

 // Output files:
    

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
    struct TreeNode *tree_position;

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
    int ID;
    int size;
    int full;
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
Node *newNode(int id, int arrival, int run, int p, int mem_size, enum STATUS stat)
{
    struct Node *tmp = (Node *)malloc(sizeof(Node));
    tmp->ID = id;
    tmp->Arrival = arrival;
    tmp->Runtime = run;
    tmp->Priority = p;
    tmp->Status = stat;
    tmp->next = NULL;
    tmp->size = mem_size;
    return tmp;
    
}

Queue *createQueue()
{
    struct Queue *q = (Queue *)malloc(sizeof(Queue));
    q->Head = NULL;
    FILE *fptr3;
    fptr3 = fopen("ttttttttttttta.log", "w");
    fprintf(fptr3, "#At time x process y state arr w total z remain y wait k \n");
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

int result = -1;
int split = 0;
void inOrder(TreeNode *root, Node *p)
{

   
    if (!root)
        return;

    inOrder(root->left, p);
    if (result == 1 || split == 1)
        return;
    inOrder(root->right, p);
    if (result == 1 || split == 1)
        return;
    if (!root->left && !root->right && root->full != 1)
    {
        split = 1;
       
        if ((root->size) / 2 < p->size)
        {
            root->full = 1;
            root->ID = p->ID;
            result = 1;
            p->tree_position = root;
            return;
        }
       
        struct TreeNode *l = (TreeNode *)malloc(sizeof(TreeNode));
        struct TreeNode *r = (TreeNode *)malloc(sizeof(TreeNode));
        l->parent = root;
        r->parent = root;
        root->left = l;
        root->right = r;
        l->size = (root->size) / 2;
        r->size = (root->size) / 2;
        r->full = 0;
        l->full = 0;

        l->start_byte = root->start_byte;
        l->end_byte = root->start_byte + l->size - 1;

        r->start_byte = root->start_byte + r->size;
        r->end_byte = (root->end_byte);
        

        if ((l->size) / 2 < p->size)
        {
            l->full = 1;
            l->ID = p->ID;
            p->tree_position = l;
            result = 1;
            
        }
        return;
    }
}


void Tree_Delete(TreeNode *root, Node*p )
{
    TreeNode * to_be_deleted = p->tree_position;
    to_be_deleted->full = 0;
    TreeNode * parent_ =  to_be_deleted->parent;
    TreeNode * check;
    if (parent_->right ==  to_be_deleted)
        check = parent_->left;
    if (parent_->left ==  to_be_deleted)
        check = parent_->right;
    
    if (check->full == 0 && !check->left && !check->right)
    {
        root->left = NULL;
        root->right = NULL;
    }
    
    
  
    
}
int Tree_Insert(TreeNode *root, Node *p)
{

    while (result == -1)
    {
        inOrder(root, p);
        split = 0;
    }
    result = -1;
    printf("\n");
    printf("end %d  ", p->tree_position->end_byte);
    printf("\n");
    printf("end %d",  p->tree_position->end_byte);
   
    return 0;
}

// int Tree_Insert(TreeNode *root, Node *p)
// {
//     // root is the root of the tree
//     // p is the process we need to insert

//     // terminating condition

//     if (root->full && root->full == 1)
//     {
//         return -1;
//     }

//     if (root->size / 2 < p->size)
//     {
//         root->process_ID = p->ID;
//         root->full = 1;
//         return 1;
//     }

//     // recursive call

//     // node has no children
//     if (!(root->left && root->right))
//     {
//         root->left = new struct TreeNode;
//         root->left->full = 0;
//         root->left->parent = root;
//         root->left->size = root->size / 2;
//         root->left->start_byte = root->start_byte;
//         root->left->end_byte = floor(root->end_byte / 2);
//     }
// }