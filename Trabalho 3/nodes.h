#ifndef NODES
#define NODES
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


//retorna o primeiro elemento de uma lista
Node* front(Node*);

//retorna o último elemento de uma lista
Node* rear(Node*);

//adiciona um no N no final de uma fila Q
void enqueue(Node** q, Node* n); 

//remove o primeiro nó, se fr = 1 da free no nó, caso contrário não da free. tipo 1 se processo, tipo 2 se burst
void dequeue(Node**, int, int);

//cria um no que armazena um PROCESSO
Node *createNodeProc(Process);

//cria um no que armazena um INTEIRO (burst)
Node *createNodeBurst(int);

//tira o primeiro elemento de Q1 e coloca no final de Q2
void switchQueue(Node** q1, Node** q2);

//encontra o elemento anterior a um no target
Node *findLast(Node* q, Node *target);

//pega o id de um processo
unsigned int getId(Node *node);

//Percorre uma fila q e retorna o primeiro elemento com submission_time <= ms
Node* getLowest(Node *q);

//percorre a lista e retorna o elemento com menor burst
Node* getLowestJob(Node *q);

//cria copia de uma lista de bursts
Node* createBurstCopy(Node* original);

//cria copia de um node process
Node *createCopy(Node* original);

//encontra o no com maior prioridade de uma lista
Node *getHighestPrio(Node*);

//libera a lista de bursts
void freeBursts(Process* process);

//libera um no do tipo (1 = PROCESS) (2 = BURST)
void freeNode(Node* node, int type);

//libera uma lista de processos
void freeList(Node* head);

//troca o no NODE da fila origin para o final da fila dest
void switchQueues(Node** origin, Node** dest, Node* node);

//puxa o processo mais proximo de entrar na CPU
int pullNextReady(Node** waiting, Node** io_waiting, Node **ready, int is_seq);

//varre uma lista buscando processos com sbm time <= ms
void checkReady(Node** queue, Node** ready, int ms);

//retorna o tamanho de uma lista
int getSize(Node* list);

#endif