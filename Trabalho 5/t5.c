/*
 * @Author: Victor Keony
 * @Description: Trabalho 5 de Sistemas Operacionais (semáforos binarios)
 * @Github: keon-y
 */

//COMPILAR COM A FLAG -pthread

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

typedef struct Node
{
    struct Node *next;
    __int32_t value;
    pthread_mutex_t lock;
    __int8_t last_operation;

} Node;

void insertNode(__int32_t value, Node **ptr)
{
    Node *new_node = (Node *)malloc(sizeof(Node));

    if (!new_node)
    {
        printf("Erro ao alocar memoria para o no com valor %d\n", value);
        exit(EXIT_FAILURE);
    }

    new_node->next = NULL;
    new_node->value = value;

    if (pthread_mutex_init(&new_node->lock, NULL) == -1) // erro ao criar semaforo
    {
        printf("\n Erro ao criar semaforo do no %d\n", value);
        exit(EXIT_FAILURE);
    }

    pthread_mutex_lock(&new_node->lock);
    new_node->last_operation = 1; // criado

    Node *runner = *ptr;

    if (!runner)
    {
        *ptr = new_node;
    }
    else
    {
        while (runner->next)
            runner = runner->next;

        runner->next = new_node;
        pthread_mutex_unlock(&runner->lock);
    }

    if (value == -2)
        pthread_mutex_unlock(&new_node->lock);
}

void deleteNode(Node *node)
{
    pthread_mutex_unlock(&node->lock);
    pthread_mutex_destroy(&node->lock);
    free(node);
}

int is_prime(int n)
{
    if (n % 2 == 0)
        return n == 2;

    for (int i = 3; i <= n / i; i += 2)
    {
        if (n % i == 0)
            return 0;
    }
    return n > 1;
}

void *thread_removePairs(void *parameters)
{
    Node **root = (Node **)parameters;

    while (!(*root))
        ;

    Node *runner = *root;
    Node *previous_node = NULL;

    while (1)
    {
        pthread_mutex_lock(&runner->lock);

        if (runner->value == -2) // condicao de parada
        {
            runner->last_operation = 2;
            pthread_mutex_unlock(&runner->lock);
            if (previous_node)
                pthread_mutex_unlock(&previous_node->lock);
            break;
        }

        if (!(runner->value & 1) && runner->value > 2) // par
        {
            // precisa adquirir o lock do proximo
            pthread_mutex_lock(&runner->next->lock);

            if (!previous_node)
                *root = runner->next;
            else
                previous_node->next = runner->next;

            Node *aux = runner;
            runner = runner->next;
            deleteNode(aux);
            pthread_mutex_unlock(&runner->lock);
            continue;
        }

        if (previous_node)
            pthread_mutex_unlock(&previous_node->lock);

        pthread_mutex_lock(&runner->next->lock);
        previous_node = runner;
        runner->last_operation = 2;
        runner = runner->next;
        pthread_mutex_unlock(&runner->lock);
    }
    pthread_exit(NULL);
}

void *thread_removeNotPrimes(void *parameters)
{
    Node **root = (Node **)parameters;

    while (!(*root) || (*root)->last_operation != 2)
        ;

    Node *runner = *root;
    Node *previous_node = NULL;

    while (1)
    {
        pthread_mutex_lock(&runner->lock);

        if (runner->value == -2) // condição de parada
        {
            runner->last_operation = 3;
            pthread_mutex_unlock(&runner->lock);

            if (previous_node)
                pthread_mutex_unlock(&previous_node->lock);
            
            break;
        }

        if (!is_prime(runner->value)) // nao eh primo
        {
            pthread_mutex_lock(&runner->next->lock); // precisa adquirir o lock do proximo pq a exclusao pode causar erros se for o ultimo elemento

            if (!previous_node)
                *root = runner->next;
            else
                previous_node->next = runner->next;

            Node *aux = runner;
            runner = runner->next;
            deleteNode(aux);
            pthread_mutex_unlock(&runner->lock);

            continue;
        }

        if (previous_node)     
            pthread_mutex_unlock(&previous_node->lock);

        pthread_mutex_lock(&runner->next->lock);
        previous_node = runner;
        runner->last_operation = 3;
        runner = runner->next;
        pthread_mutex_unlock(&runner->lock);
    }

    pthread_exit(NULL);
}

void *thread_printList(void *parameters)
{
    Node **root = (Node **)parameters;

    while (!(*root) || (*root)->last_operation != 3)
        ;

    Node *runner = *root;

    while (1)
    {

        pthread_mutex_lock(&runner->lock);

        if (runner->value == -2) // condicao de parada
        {
            deleteNode(runner);
            break;
        }

        printf("%d ", runner->value);

        pthread_mutex_lock(&runner->next->lock);
        Node *aux = runner;
        runner = runner->next;
        deleteNode(aux);
        pthread_mutex_unlock(&runner->lock);
    }

    pthread_exit(NULL);
}

int main(int argc, char **argv)
{

    if (argc != 2)
    {
        printf("Erro! uso do comando: %s <nome do arquivo>", argv[0]);
        exit(EXIT_FAILURE);
    }

    FILE *file_ptr;
    if ((file_ptr = fopen(argv[1], "r")) == NULL)
    {
        perror("Arquivo nao encontrado");
        exit(EXIT_FAILURE);
    }

    int read_value;
    Node *list = NULL;
    Node *list_tail = NULL;

    pthread_t t_removePairs, t_removeNotPrimes, t_printList;

    pthread_create(&t_removePairs, NULL, thread_removePairs, &list);
    pthread_create(&t_removeNotPrimes, NULL, thread_removeNotPrimes, &list);
    pthread_create(&t_printList, NULL, thread_printList, &list);

    while (fscanf(file_ptr, "%d", &read_value) != EOF)
    {
        if (!list_tail)
        {
            insertNode(read_value, &list);
            list_tail = list;
        }
        else
        {
            insertNode(read_value, &list_tail);
            list_tail = list_tail->next;
        }
    }

    insertNode(-2, &list_tail); // avisar que acabou

    pthread_join(t_removePairs, NULL);
    pthread_join(t_removeNotPrimes, NULL);
    pthread_join(t_printList, NULL);

    return 0;
}