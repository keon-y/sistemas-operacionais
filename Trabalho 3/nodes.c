#include <stdio.h>
#include <stdlib.h>

//FILA ENCADEADA
typedef struct Node {
    void* data;
    struct Node *next;
} Node;

typedef struct Process {
    unsigned int id;
    unsigned int priority; //prioridade
    int submission_time_ms; //tempo de admissao
    Node* cpu_bursts; //lista encadeada de picos de CPU e ES
} Process;

void freeBursts(Process* process) {
    Node* head = process->cpu_bursts;
    while(head) {
        Node* temp = head;
        head = head->next;
        free(temp->data);
        free(temp);
    }
}
void freeNode(Node* node, int type) {
    if (node) {
        if (node->data) {
            if (type == 1) {
                Process* process = (Process*)node->data;
                freeBursts(process); 
                free(process); 
            }
            else if (type == 2) {
                int* data = (int*)node->data;
                free(data);
            }
        }
        free(node);
    }
}


Node *createNodeProc(Process p) {
    Node* node = malloc(sizeof(Node));
    if (!node) return NULL; //ERRO DE ALOCACAO
    node->data = malloc(sizeof(Process));
    if(!node->data) {
        free(node);
        return NULL;
    }
    *(Process*)node->data = p;
    node->next = NULL;
    return node;
}

Node *createNodeBurst(int b) {
    Node* node = malloc(sizeof(Node));
    if (!node) return NULL;
    node->data = malloc(sizeof(int));
    if(!node->data) {
        free(node);
        return NULL;
    }
    *(int*)node->data = b;
    node->next = NULL;
    return node;
}

Node* front(Node* queue) {
    if (!queue) return NULL;
    return queue;
} 
Node* rear(Node* queue){
    if(!queue) return NULL;
    Node *ptr = queue;
    while(ptr->next != NULL) 
        ptr = ptr->next;
    
    return ptr;
}

void enqueue(Node** queue, Node* node){
    if (!*queue) {//fila vazia
        *queue = node;
        (*queue)->next = NULL;
        return;
    }
    rear(*queue)->next = node;
    node->next = NULL;
} 
void dequeue(Node** queue, int fr, int type) {
    if(!*queue) return;
    Node *temp = (*queue)->next;
    (*queue)->next = NULL;
    if (fr)
        freeNode(*queue, type);
    *queue = temp;
}

void switchQueue(Node** q1, Node** q2) {
    Node *temp = *q1;
    dequeue(q1, 0, 1);
    enqueue(q2, temp);
}


Node *findLast(Node* q, Node *target) {
    Node *last = NULL;
    while (q && q != target){
        last = q;
        q = q->next;
    }
    return last;
}
unsigned int getId(Node *node){
    if (!node || !node->data) return -1;
    return (*(Process*) node->data).id;
}
Node* getLowest(Node *q) {
    Node *lowest = NULL;
    while (q) {
        if (!lowest) lowest = q;
        else if ( ( (Process*)q->data)->submission_time_ms <= ((Process*)lowest->data)->submission_time_ms)
            lowest = q;
        q = q->next;
    }
    return lowest;
}

Node* getLowestJob(Node *q) {
    Node *lowest = NULL;
    while (q) {
        if (!lowest) lowest = q;
        else if ( *(int*)(((Process*)q->data)->cpu_bursts->data) <= *(int*)(((Process*)lowest->data)->cpu_bursts->data)) {
            lowest = q;
        }
        q = q->next;
    }
    return lowest;
}

Node* getHighestPrio(Node *q) {
    Node *highest = NULL;
    while (q) {
        if (!highest) highest = q;
        else if ( ( (Process*)q->data)->priority > ((Process*)highest->data)->priority)
            highest = q;
        q = q->next;
    }
    return highest;
}


void freeList(Node* head) {
    Node* temp;
    while (head) {
        temp = head;
        head = head->next;
        freeNode(temp, 1); 
    }
}

Node* createBurstCopy(Node* original) {
    if (!original) return NULL;

    int* original_burst = (int*)original->data;
    int* new_burst = (int*)malloc(sizeof(int));
    if (!new_burst) return NULL;
    
    *new_burst = *original_burst;
    
    Node* new_node = (Node*)malloc(sizeof(Node));
    if (!new_node) {
        free(new_burst);
        return NULL;
    }
    
    new_node->data = new_burst;
    new_node->next = createBurstCopy(original->next);
    
    return new_node;
}

Node* createCopy(Node* original) {
    if (!original) return NULL;

    Process* original_proc = (Process*)original->data;
    Process* new_proc = (Process*)malloc(sizeof(Process));
    if (!new_proc) return NULL;
    
    new_proc->id = original_proc->id;
    new_proc->priority = original_proc->priority;
    new_proc->submission_time_ms = original_proc->submission_time_ms;
    
    new_proc->cpu_bursts = createBurstCopy(original_proc->cpu_bursts);
    if (!new_proc->cpu_bursts && original_proc->cpu_bursts) {
        free(new_proc);
        return NULL;
    }

    Node* new_node = (Node*)malloc(sizeof(Node));
    if (!new_node) {
        freeList(new_proc->cpu_bursts);
        free(new_proc);
        return NULL;
    }

    new_node->data = new_proc;
    new_node->next = createCopy(original->next);
    if (!new_node->next && original->next) {
        freeNode(new_node, 1);
        return NULL;
    }

    return new_node;
}


void switchQueues(Node** origin, Node** dest, Node* node) {
    Node *l = findLast(node, *origin);
    if (l)
        l->next = node->next;
    else if (*origin == node) *origin = (*origin)->next;
    node->next = NULL;
    if (*dest)
        enqueue(dest, node);
    else freeNode(node, 1);               
}

int pullNextReady(Node** waiting, Node** io_waiting, Node **ready, int is_seq) {
    int time;
    Node *temp = is_seq ? *io_waiting : getLowest(*io_waiting);
    Node *temp2 = getLowest(*waiting);
    if (temp && temp2)
        temp = ((Process *)temp->data)->submission_time_ms < ((Process *)temp2->data)->submission_time_ms ? temp : temp2;
    else if (!temp && temp2)
        temp = temp2;

    if (temp) //temp = menor tempo entre io_waiting e waiting
    {
        time = ((Process *)temp->data)->submission_time_ms;
        if (temp != *waiting && temp == temp2) //ta no meio da fila waiting
            findLast(*waiting, temp)->next = temp->next;
        else if (temp == *waiting) //ta no comeco da fila waiting
            *waiting = (*waiting)->next;
        else if (temp == *io_waiting) //ta no comeco da fila io_waiting
            *io_waiting = (*io_waiting)->next;
        else //ta no meio da fila io_waiting
            findLast(*io_waiting, temp)->next = temp->next;
        temp->next = NULL;
        enqueue(ready, temp);
    }
    return time;
}

void checkReady(Node** queue, Node** ready, int ms) {
    Node *temp = getLowest(*queue);
    while (temp && ((Process*)temp->data)->submission_time_ms <= ms) {
        if(temp != *queue)
            findLast(*queue, temp)->next = temp->next;
        else *queue = (*queue)->next; //nunca vai ser NULL pq temp vem de *queue
        temp->next = NULL;
        enqueue(ready, temp);
        temp = getLowest(*queue);
    }
}

int getSize(Node* list) {
    int size = 0;
    while (list) {
        size++;
        list = list->next;
    }
    return size;
}