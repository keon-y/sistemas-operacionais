#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>

//lista duplamente encadeada (implementacao eficiente de LRU)
typedef struct Node {
    int value;
    struct Node *next, *prev;
} Node;


//algoritmos
void FIFO(FILE*, int*, size_t, size_t, size_t);
void LRU(FILE*, int*, size_t, size_t, size_t);
void OPT(FILE*, int*, size_t, size_t, size_t);

//auxiliares
int exists(int*, size_t, int);
void* initializeArray(int, size_t);
int getOptimal(int*, int*, size_t, size_t, size_t, size_t);

void insertNode(Node**, Node*);
Node* linkedListSearch(Node*, int);
void popTail(Node**);
Node* createNode(int val);

int main(int argc, char **argv)
{

    if (argc != 4)
    {
        printf("Usage: %s <page size> <memory size> <input filename>\n", argv[0]);
        return -1;
    }

    FILE *file_ptr = fopen(argv[3], "r");
    if (!file_ptr)
    {
        printf("File '%s' not found!\n", argv[3]);
        return -1;
    }

    // descobrir o tamanho do arquivo para alocar memória
    fseek(file_ptr, 0L, SEEK_END);
    long file_size = ftell(file_ptr) / sizeof(unsigned short);
    fseek(file_ptr, 0L, SEEK_SET);

    // acessos
    int *page_accesses = malloc(sizeof(int) * file_size);
    size_t accesses_current_size = 0;
    int read;

    // paginas
    size_t frames = atoi(argv[2]) / atoi(argv[1]); //quadros
    size_t page_size = atoi(argv[1]);

    while (fscanf(file_ptr, "%d", &read) != EOF)
    {
        page_accesses[accesses_current_size++] = read;
    }

    fclose(file_ptr);

    file_ptr = fopen("erros.out", "w");

    fprintf(file_ptr, "FIFO:\n");
    FIFO(file_ptr, page_accesses, accesses_current_size, page_size, frames);
    fprintf(file_ptr, "\nOPT:\n");
    OPT(file_ptr, page_accesses, accesses_current_size, page_size, frames);
    fprintf(file_ptr, "\nLRU:\n");
    LRU(file_ptr, page_accesses, accesses_current_size, page_size, frames);

    fclose(file_ptr);

    free(page_accesses);

    return 0;
}

void FIFO(FILE* fp, int *page_accesses, size_t accesses_size, size_t page_size, size_t frame_size)
{
    size_t page_faults = 0; // auxiliar para saber sempre onde ta o primeiro
    int* pages = (int*) initializeArray(sizeof(int), frame_size);
    for (int i = 0; i < accesses_size; i++)
    {
        int page = page_accesses[i] / page_size;
        if (exists(pages, frame_size, page) == -1)
        {
            pages[page_faults % frame_size] = page;
            fprintf(fp, "erro de pagina endereço :%d pagina: %d\n", page_accesses[i], page);
            page_faults++;
        }
    }

    free(pages);
    printf("FIFO: %ld erros (%.2f%%)\n", page_faults, (page_faults * 100.0) / accesses_size);
}

void LRU(FILE* fp, int *page_accesses, size_t accesses_size, size_t page_size, size_t frame_size)
{
    Node* pages = NULL;
    size_t page_faults = 0;

    for (int i = 0; i < accesses_size; i++)
    {
        int page = page_accesses[i] / page_size;
        Node* hit = linkedListSearch(pages, page);
        if (!hit)
        {
            if(page_faults >= frame_size) //capacidade maxima, precisa remover o menos recentemente usado
                popTail(&pages);
            insertNode(&pages, createNode(page));
            fprintf(fp, "erro de pagina endereço :%d pagina: %d\n", page_accesses[i], page);
            page_faults++;
        }
        else insertNode(&pages, hit);   
    }
    while(pages) popTail(&pages);
    printf("OPT: %ld erros (%.2f%%)\n", page_faults, (page_faults * 100.0) / accesses_size);


}

void OPT(FILE* fp, int *page_accesses, size_t accesses_size, size_t page_size, size_t frame_size) {
    int* pages = (int*) initializeArray(sizeof(int), frame_size);
    size_t page_faults = 0;
    for (int i = 0; i < accesses_size; i++)
    {
        int page = page_accesses[i] / page_size;
        if (exists(pages, frame_size, page) == -1)
        {
            pages[getOptimal(page_accesses, pages, accesses_size, page_size, i, frame_size)] = page;
            fprintf(fp, "erro de pagina endereço :%d pagina: %d\n", page_accesses[i], page);
            page_faults++;
        }
    }
    free(pages);
    printf("LRU: %ld erros (%.2f%%)\n", page_faults, (page_faults * 100.0) / accesses_size);

}

// funções auxiliares

// shouldCount for 0 ele para a execução assim que encontra o target
// apenas algumas chamadas precisam contar quantos elementos existem
int exists(int *array, size_t array_size, int target)
{
    for (int i = 0; i < array_size; i++)
    {
        if (array[i] == target)
            return i;
    }
    return -1;
}

Node* linkedListSearch(Node* list_ptr, int val) {
    if (!list_ptr) return NULL;
    while (list_ptr) {
        if (list_ptr->value == val)
            return list_ptr;
        list_ptr = list_ptr->next;
    }
    return NULL;
}

void* initializeArray(int bytes, size_t size) {
    void* arr = malloc(bytes * size);
    if (arr == NULL) {
        perror("Erro de alocacao");
        exit(1);
    }

    for (int i = 0; i < size; i++) {
        ((int*)arr)[i] = -1;
    }
    return arr;
}

Node* createNode(int val) {
    Node* n = malloc(sizeof(Node));
    if(!n) {printf("Erro de alocacao"); exit(1);}
    n->value = val; n->next = NULL; n->prev = NULL;
    return n;
} 

//insere no COMECO
void insertNode(Node** head, Node* node) {
    if (! *head ) {
        *head = node;
        return;
    }

    if (*head == node) return;

    if (node->prev) {
        node->prev->next = node->next;
    }
    if(node->next) {
        node->next->prev = node->prev;
    }
     
    //insere no comeco
    node->next = *head;
    node->prev = NULL;
    (*head)->prev = node;
    *head = node;

}

void popTail(Node** head) {
    if (!*head) return; 
    Node* tail = *head;
    while (tail->next) tail = tail->next;
    if (tail->prev) tail->prev->next = NULL;
    else *head = NULL;
    free(tail);
}

//acha o indice optimo para substituir
int getOptimal(int* page_accesses, int* pages, size_t accesses_size, size_t page_size, size_t access_index, size_t frame_size) {
    for (int i = 0; i < frame_size; i++) {
        if (pages[i] == -1) return i;  
    }

    int frequency[frame_size];
    memset(frequency, 0, sizeof(int) * frame_size);
    int smallest = 0;

    for (int i = 0; i < frame_size; i++) {
        int index = -1;
        
        for (int j = access_index; j < accesses_size; j++) {
            int target = page_accesses[j] / page_size; 
            if ( target == pages[i] ) {
                frequency[i]++;
            }
        }
        if (frequency[i] < frequency[smallest])
            smallest = i;
    }


    return smallest;
}