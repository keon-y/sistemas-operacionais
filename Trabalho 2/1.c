#include <pthread.h>
#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdint.h>

int *number_array;

void *sum_array_thread(void *param);

typedef struct ThreadParam {
    unsigned int start;
    unsigned int finish;
} ThreadParam;

int main(int argc, char **argv) {
    if (argc != 3) {
        printf("Uso: %s <arquivo> <numero de threads>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int thread_amount = atoi(argv[2]);
    if (thread_amount <= 0) {
        printf("Numero de threads deve ser positivo\n");
        exit(EXIT_FAILURE);
    }

    FILE *file_ptr;
    if ((file_ptr = fopen(argv[1], "rb")) == NULL) {
        perror("Arquivo nao encontrado");
        exit(EXIT_FAILURE);
    }

    struct stat st;
    if (fstat(fileno(file_ptr), &st) == -1) {
        perror("Falha ao obter o tamanho do arquivo");
        fclose(file_ptr);
        exit(EXIT_FAILURE);
    }

    size_t size = st.st_size;
    int num_numbers = size / sizeof(int);
    number_array = (int*) malloc(size);
    if (number_array == NULL) {
        perror("Falha ao alocar memoria");
        fclose(file_ptr);
        exit(EXIT_FAILURE);
    }

    if (fread(number_array, sizeof(int), num_numbers, file_ptr) != num_numbers) {
        perror("Falha ao ler arquivo");
        free(number_array);
        fclose(file_ptr);
        exit(EXIT_FAILURE);
    }
    fclose(file_ptr);

    pthread_t *tid = malloc(thread_amount * sizeof(pthread_t));
    ThreadParam *params = malloc(thread_amount * sizeof(ThreadParam));
    if (tid == NULL || params == NULL) {
        perror("Falha ao alocar memoria para threads");
        free(number_array);
        exit(EXIT_FAILURE);
    }

    intptr_t total = 0;
    unsigned int chunk_size = num_numbers / thread_amount; //quanto cada bloco vai ler
    unsigned int remainder = num_numbers % thread_amount; //sobra (no max chunk_size - 1)

    for (int i = 0; i < thread_amount; i++) {
        params[i].start = i * chunk_size;
        params[i].finish = (i + 1) * chunk_size - 1;
        if (i == thread_amount - 1) { //pode ser lidado de varias formas, mas por questoes de simplicidade
            params[i].finish += remainder; //a ultima thread vai lidar com o valor restante
        } //outro jeito de fazer isso eh dividindo igualmente +1 para todos, no pior caso apenas a ultima thread nao vai ter +1

        pthread_create(&tid[i], NULL, sum_array_thread, &params[i]);
    }

    for (int i = 0; i < thread_amount; i++) {
        void *t;
        pthread_join(tid[i], &t);
        total += (intptr_t)t;
    }

    printf("Total: %d\n", (int)total); //converter para int (pq o exercicio pede com overflow)
    
    free(tid);
    free(params);
    free(number_array);

    return 0;
}

void *sum_array_thread(void *param) {
    ThreadParam *p = (ThreadParam*)param;
    intptr_t total = 0;
    for (int i = p->start; i <= p->finish; i++) {
        total += number_array[i];
    }
    pthread_exit((void*)total);
}
