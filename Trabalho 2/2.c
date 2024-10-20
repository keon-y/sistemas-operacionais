#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

typedef struct {
    int *numbers;
    int start;
    int end;
    int *result;
} ProcessArgs;

void sum_subarray(ProcessArgs data) {
    int sum = 0;
    for (int i = data.start; i < data.end; i++) {
        sum += data.numbers[i];
    }
    *(data.result) = sum;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Uso: %s <arquivo> <numero de processos>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int num_processes = atoi(argv[2]);
    if (num_processes <= 0) {
        printf("Numero de processos deve ser positivo");
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
    int *number_array = (int*) malloc(size);

    if (fread(number_array, sizeof(int), num_numbers, file_ptr) != num_numbers) {
        perror("Falha ao ler arquivo");
        free(number_array);
        fclose(file_ptr);
        exit(EXIT_FAILURE);
    }
    fclose(file_ptr);

    // memoria compartilhada dos resultados
    int shm_id = shmget(IPC_PRIVATE, num_processes * sizeof(int), IPC_CREAT | 0666);
    if (shm_id == -1) {
        perror("falha no shmget");
        free(number_array);
        exit(EXIT_FAILURE);
    }

    int *results = (int*) shmat(shm_id, NULL, 0);
    if (results == (void*) -1) {
        perror("falha no shmat");
        free(number_array);
        shmctl(shm_id, IPC_RMID, NULL);
        exit(EXIT_FAILURE);
    }

    int chunk_size = num_numbers / num_processes;
    for (int i = 0; i < num_processes; i++) {
        if (fork() == 0) { //filho
            ProcessArgs data;
            data.numbers = number_array;
            data.start = i * chunk_size;
            data.end = (i == num_processes - 1) ? num_numbers : (i + 1) * chunk_size;
            data.result = &results[i];
            sum_subarray(data);
            shmdt(results); //dettach
            exit(0);
        }
    }

    //esperar filhos
    for (int i = 0; i < num_processes; i++) {
        wait(NULL);
    }

    //calcular soma
    int total = 0;
    for (int i = 0; i < num_processes; i++) {
        total += results[i];
    }
    printf("Total: %d\n", total);

    free(number_array);
    shmdt(results);
    shmctl(shm_id, IPC_RMID, NULL);

    return 0;
}
