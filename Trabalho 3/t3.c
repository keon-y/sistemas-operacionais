#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "nodes.h"


void RR(Node* list, int quantum, int is_seq, FILE* fptr) {
    Node *waiting = list;
    Node *io_waiting = NULL;
    Node *ready = NULL;
    Node *lastExec = NULL;
    int ms = 0;
    int first = 1;
    int list_size = getSize(list);

    int inactive_ms = 0;
    int proc_completed = 0;
    int first_sbm_ms[list_size];//alocar um array para guardar os primeiros tempos de submissao
    int wait_time_ms[list_size]; //alocar um array para guardar a espera total de cada processo
    int turnaround_time_ms[list_size]; //alocar um array para guardar a espera total de cada processo
    Node *ptr = waiting;
    for (int i = 0; i < list_size; i++) {
        first_sbm_ms[i] = ((Process*)ptr->data)->submission_time_ms;
        wait_time_ms[i] = 0;
        turnaround_time_ms[i] = 0;
        ptr = ptr->next;
    }
    fprintf(fptr, "[ROUND ROBIN (%dms quantum)] GANTT: ", quantum);
    while (waiting || ready || io_waiting) { //enquanto tiver processos
        
        checkReady(&waiting, &ready, ms);
        checkReady(&io_waiting, &ready, ms);
        

        // se nao tiver prontos, pega o primeiro da lista de espera e soma o tempo
        if (!ready) {
            int time = pullNextReady(&waiting, &io_waiting, &ready, is_seq);
            inactive_ms += time - ms;  
            ms = time;
            if (first) {
                fprintf(fptr, "%d[", ms);
                first = 0;
            }
            else fprintf(fptr, "*** %d|", ms);
        }
        if (lastExec) enqueue(&ready, lastExec);
        // executa o processo
        Process *p = (Process*) ready->data;
        int index = p->id - 1;
        wait_time_ms[index] += ms - p->submission_time_ms;
        int throughput_ms = (*(int*) (p->cpu_bursts)->data);
        if (throughput_ms <= quantum) {//se tempo de burst for menor que rr, apenas executar
            ms += throughput_ms;
            fprintf(fptr, "P%d %d|", p->id, ms);
            // tira o burst da lista
            dequeue(&p->cpu_bursts, 1, 2);

            if (p->cpu_bursts) { // se tiver outro burst, significa que tem pico de ES
                p->submission_time_ms = (*(int*) front(p->cpu_bursts)->data) + ms; // tempo de resubmissao = ms + tempo de pico de ES
                if (is_seq && io_waiting) p->submission_time_ms += ((Process*)rear(io_waiting)->data)->submission_time_ms - ms;
                switchQueue(&ready, &io_waiting);
                dequeue(&p->cpu_bursts, 1, 2);
            } else { // se nao tiver mais, dar dequeue pois acabou o processo
                dequeue(&ready, 1, 1);
                proc_completed++;
                turnaround_time_ms[index] = ms - first_sbm_ms[index];
            }
            lastExec = NULL;
        }
        else {
            ms += quantum;
            Process *tempW = (Process*) getLowest(waiting);
            Process *tempIO = (Process*) getLowest(io_waiting);
            if ( (io_waiting || waiting) && ( ready->next || (tempW && tempW->submission_time_ms <= ms) || (tempIO && tempIO->submission_time_ms <= ms) )) //checa pra ver se ele nao esta sozinho ou se vai ter um novo
                fprintf(fptr, "P%d %d|", p->id, ms);
            *(int*) ((p->cpu_bursts)->data) -= quantum;
            lastExec = ready;
            dequeue(&ready, 0, 1);
        }

    }
    float media_espera = 0.0f;
    float media_turnaround = 0.0f;
    fprintf(fptr, "]\n");
    fprintf(fptr, "[ROUND ROBIN (%dms quantum)] CPU %%: %.2f%%\n", quantum, 100 - 100 * ((float)inactive_ms/ms));
    fprintf(fptr, "[ROUND ROBIN (%dms quantum)] Throughtput: %.2f processos/segundo\n", quantum, proc_completed * (1000.0f / ms));
    for (int i = 0; i < list_size; i++) {
        fprintf(fptr, "[ROUND ROBIN (%dms quantum)] P%d Espera: %dms\n", quantum, i + 1, wait_time_ms[i]);
        fprintf(fptr, "[ROUND ROBIN (%dms quantum)] P%d Turnaround: %dms\n", quantum, i + 1, turnaround_time_ms[i]);
        media_espera += wait_time_ms[i];
        media_turnaround += turnaround_time_ms[i];
    }
    fprintf(fptr, "[ROUND ROBIN (%dms quantum)] MEDIA TEMPO DE ESPERA: %.2fms", quantum, media_espera / list_size);
    fprintf(fptr, "\n[ROUND ROBIN (%dms quantum)] MEDIA TEMPO DE TURNAROUND: %.2fms\n", quantum, media_turnaround / list_size);
}

void PP(Node* list, int is_seq, FILE *fptr) {
    Node *waiting = list;
    Node *ready = NULL;
    Node *io_waiting = NULL;
    int ms = 0;
    int first = 1;
    int list_size = getSize(list);

    int inactive_ms = 0;
    int proc_completed = 0;
    int first_sbm_ms[list_size];//alocar um array para guardar os primeiros tempos de submissao
    int wait_time_ms[list_size]; //alocar um array para guardar a espera total de cada processo
    int turnaround_time_ms[list_size]; //alocar um array para guardar a espera total de cada processo
    Node *ptr = waiting;
    for (int i = 0; i < list_size; i++) {
        first_sbm_ms[i] = ((Process*)ptr->data)->submission_time_ms;
        wait_time_ms[i] = 0;
        turnaround_time_ms[i] = 0;
        ptr = ptr->next;
    }
    fprintf(fptr,"[PRIORIDADE PREEMPTIVO] GANTT: ");
    while (waiting || ready || io_waiting) { // enquanto tiver processos
         if (!first) fprintf(fptr,"|");
        
        checkReady(&waiting, &ready, ms);
        checkReady(&io_waiting, &ready, ms);
        

        // se nao tiver prontos, pega o primeiro da lista de espera e soma o tempo
        if (!ready) {
            int time = pullNextReady(&waiting, &io_waiting, &ready, is_seq);
            inactive_ms += time - ms;  
            ms = time;
            if (first) {
                fprintf(fptr,"%d[", ms);
                first = 0;
            }
            else fprintf(fptr,"*** %d|", ms);
        }

        // executa o processo
        Node *n = getHighestPrio(ready);
        int index = getId(n) - 1;

        Process *p = (Process*)n->data;
        wait_time_ms[index] += ms - p->submission_time_ms;
        int throughput_ms = (*(int*) ( (p->cpu_bursts)->data));
        
        Node *temp = NULL;
        if (io_waiting && waiting)
            temp = ((Process*)getLowest(waiting)->data)->submission_time_ms < ((Process*)getLowest(io_waiting)->data)->submission_time_ms ? getLowest(waiting) : getLowest(io_waiting);
        else if (waiting && !io_waiting)
            temp = getLowest(waiting);
        else if (!waiting && io_waiting)
            temp = getLowest(io_waiting);
    
        if (temp && ((Process*)temp->data)->submission_time_ms < ms + throughput_ms && ((Process*)temp->data)->priority > p->priority) {
            *(int*)p->cpu_bursts->data -= ((Process*) temp->data)->submission_time_ms - ms;
            ms = ((Process*) temp->data)->submission_time_ms;
            fprintf(fptr,"P%d %d", p->id, ms);
        } else {
            ms += throughput_ms; 
            fprintf(fptr,"P%d %d", p->id, ms);
            // tira o burst da lista
            dequeue(&p->cpu_bursts, 1, 2);

            Node *l = findLast(ready, n);

            if (p->cpu_bursts) { // se tiver outro burst, significa que tem pico de ES
                p->submission_time_ms = *(int*) p->cpu_bursts->data + ms; // tempo de resubmissao = ms + tempo de pico de ES
                if (is_seq && io_waiting) p->submission_time_ms += ((Process*)rear(io_waiting)->data)->submission_time_ms - ms;
                if (l)
                    l->next = n->next;
                else if (ready) ready = ready->next;
                enqueue(&io_waiting, n);
                dequeue(&p->cpu_bursts, 1, 2);
            } else { // se nao tiver mais, dar dequeue pois acabou o processo
                if (l) l->next = n->next;
                else if (ready) ready = ready->next;
                freeNode(n, 1);
                proc_completed++;
                turnaround_time_ms[index] = ms - first_sbm_ms[index];
            }
        }
    }
    float media_espera = 0.0f;
    float media_turnaround = 0.0f;
    fprintf(fptr,"]\n");
    fprintf(fptr,"[Prioridade Preemptivo] CPU %%: %.2f%%\n", 100 - 100 * ((float)inactive_ms/ms));
    fprintf(fptr,"[Prioridade Preemptivo] Throughtput: %.2f processos/segundo\n", proc_completed * (1000.0f / ms));
    for (int i = 0; i < list_size; i++) {
        fprintf(fptr,"[Prioridade Preemptivo] P%d Espera: %dms\n", i + 1, wait_time_ms[i]);
        fprintf(fptr,"[Prioridade Preemptivo] P%d Turnaround: %dms\n", i + 1, turnaround_time_ms[i]);
        media_espera += wait_time_ms[i];
        media_turnaround += turnaround_time_ms[i];
    }
    fprintf(fptr,"[Prioridade Preemptivo] MEDIA TEMPO DE ESPERA: %.2fms", media_espera / list_size);
    fprintf(fptr,"\n[Prioridade Preemptivo] MEDIA TEMPO DE TURNAROUND: %.2fms\n", media_turnaround / list_size);
}

void SRTF(Node* list, int is_seq, FILE* fptr) {
    Node *waiting = list;
    Node *ready = NULL;
    Node *io_waiting = NULL;
    int ms = 0;
    int first = 1;
    int list_size = getSize(list);

    int inactive_ms = 0;
    int proc_completed = 0;
    int first_sbm_ms[list_size];//alocar um array para guardar os primeiros tempos de submissao
    int wait_time_ms[list_size]; //alocar um array para guardar a espera total de cada processo
    int turnaround_time_ms[list_size]; //alocar um array para guardar a espera total de cada processo
    Node *ptr = waiting;
    for (int i = 0; i < list_size; i++) {
        first_sbm_ms[i] = ((Process*)ptr->data)->submission_time_ms;
        wait_time_ms[i] = 0;
        turnaround_time_ms[i] = 0;
        ptr = ptr->next;
    }
    fprintf(fptr,"[SRTF] GANTT: ");
    while (waiting || ready || io_waiting) { // enquanto tiver processos
         if (!first) fprintf(fptr,"|");
        
        checkReady(&waiting, &ready, ms);
        checkReady(&io_waiting, &ready, ms);
        

        // se nao tiver prontos, pega o primeiro da lista de espera e soma o tempo
        if (!ready) {
            int time = pullNextReady(&waiting, &io_waiting, &ready, is_seq);
            inactive_ms += time - ms;  
            ms = time;
            if (first) {
                fprintf(fptr,"%d[", ms);
                first = 0;
            }
            else fprintf(fptr,"*** %d|", ms);
        }


        // executa o processo
        Node *n = getLowestJob(ready);
        int index = getId(n) - 1;

        Process *p = (Process*)n->data;
        wait_time_ms[index] += ms - p->submission_time_ms;
        int throughput_ms = (*(int*) (p->cpu_bursts->data));
        
        Node *candidate = NULL;

        Node *templist = createCopy(waiting); // CRIAR UMA LISTA TEMPORARIA DE CANDIDATOS (REMOVER DE WAITING PRA NAO CAUSAR LOOP INFINITO)
        Node *temp = NULL;
        if(templist) temp = getLowest(templist);
        // checa todos da lista de espera
        while (temp && ((Process*)temp->data)->submission_time_ms < ms + throughput_ms) {
            if ( ((Process*)temp->data)->submission_time_ms + (*(int*)((Process*)temp->data)->cpu_bursts->data) < ms + throughput_ms ) {
                if (!candidate || ((Process*)temp->data)->submission_time_ms + (*(int*)((Process*)temp->data)->cpu_bursts->data) < ((Process*)candidate->data)->submission_time_ms + (*(int*)((Process*)candidate->data)->cpu_bursts->data)) {
                    if(candidate) freeNode(candidate, 1);
                    candidate = temp;
                }
            }
            if (temp != templist)
                findLast(templist, temp)->next = temp->next;
            else templist = templist->next;

            temp = getLowest(templist);
        }

        freeList(templist);
        templist = createCopy(io_waiting);
        temp = NULL;
        if(templist) temp = getLowest(templist);

        while (temp && ((Process*)temp->data)->submission_time_ms <= ms + throughput_ms) {
            if ( ((Process*)temp->data)->submission_time_ms + (*(int*)((Process*)temp->data)->cpu_bursts->data) < ms + throughput_ms ) {
                if (!candidate || ((Process*)temp->data)->submission_time_ms + (*(int*)((Process*)temp->data)->cpu_bursts->data) < ((Process*)candidate->data)->submission_time_ms + (*(int*)((Process*)candidate->data)->cpu_bursts->data)) {
                    if(candidate) freeNode(candidate, 1);
                    candidate = temp;
                }
            }
            if (temp != templist)
                findLast(templist, temp)->next = temp->next;
            else templist = templist->next;

            temp = getLowest(templist);
        }

        freeList(templist);


        if (candidate) {
            *(int*)p->cpu_bursts->data -= ((Process*) candidate->data)->submission_time_ms - ms;
            ms = ((Process*) candidate->data)->submission_time_ms;
            fprintf(fptr,"P%d %d", p->id, ms);
        } else {
            ms += throughput_ms; 
            fprintf(fptr,"P%d %d", p->id, ms);
            // tira o burst da lista
            dequeue(&p->cpu_bursts, 1, 2);

            Node *l = findLast(ready, n);

            if (p->cpu_bursts) { // se tiver outro burst, significa que tem pico de ES
                p->submission_time_ms = *(int*) p->cpu_bursts->data + ms; // tempo de resubmissao = ms + tempo de pico de ES
                if (is_seq && io_waiting) p->submission_time_ms += ((Process*)rear(io_waiting)->data)->submission_time_ms - ms;
                if (l)
                    l->next = n->next;
                else if (ready) ready = ready->next;
                enqueue(&io_waiting, n);
                dequeue(&p->cpu_bursts, 1, 2);
            } else { // se nao tiver mais, dar dequeue pois acabou o processo
                if (l) l->next = n->next;
                else if (ready) ready = ready->next;
                freeNode(n, 1);
                proc_completed++;
                turnaround_time_ms[index] = ms - first_sbm_ms[index];
            }
        }
        if (candidate) freeNode(candidate, 1);
    }
    float media_espera = 0.0f;
    float media_turnaround = 0.0f;
    fprintf(fptr,"]\n");
    fprintf(fptr,"[SRTF] CPU %%: %.2f%%\n", 100 - 100 * ((float)inactive_ms/ms));
    fprintf(fptr,"[SRTF] Throughtput: %.2f processos/segundo\n", proc_completed * (1000.0f / ms));
    for (int i = 0; i < list_size; i++) {
        fprintf(fptr,"[SRTF] P%d Espera: %dms\n", i + 1, wait_time_ms[i]);
        fprintf(fptr,"[SRTF] P%d Turnaround: %dms\n", i + 1, turnaround_time_ms[i]);
        media_espera += wait_time_ms[i];
        media_turnaround += turnaround_time_ms[i];
    }
    fprintf(fptr,"[SRTF] MEDIA TEMPO DE ESPERA: %.2fms", media_espera / list_size);
    fprintf(fptr,"\n[SRTF] MEDIA TEMPO DE TURNAROUND: %.2fms\n", media_turnaround / list_size);
}

void SJF(Node* list, int is_seq, FILE *fptr) {
    Node *waiting = list;
    Node *ready = NULL;
    Node *io_waiting = NULL;
    int ms = 0;
    int first = 1;

    int list_size = getSize(list);

    int inactive_ms = 0;
    int proc_completed = 0;
    int first_sbm_ms[list_size];//alocar um array para guardar os primeiros tempos de submissao
    int wait_time_ms[list_size]; //alocar um array para guardar a espera total de cada processo
    int turnaround_time_ms[list_size]; //alocar um array para guardar a espera total de cada processo
    Node *ptr = waiting;
    for (int i = 0; i < list_size; i++) {
        first_sbm_ms[i] = ((Process*)ptr->data)->submission_time_ms;
        wait_time_ms[i] = 0;
        turnaround_time_ms[i] = 0;
        ptr = ptr->next;
    }
    fprintf(fptr,"[SJF] GANTT: ");
    while (waiting || ready || io_waiting) { //enquanto tiver processos
        if (!first) fprintf(fptr,"|");
        
        checkReady(&waiting, &ready, ms);
        checkReady(&io_waiting, &ready, ms);
        

        // se nao tiver prontos, pega o primeiro da lista de espera e soma o tempo
        if (!ready) {
            int time = pullNextReady(&waiting, &io_waiting, &ready, is_seq);
            inactive_ms += time - ms;  
            ms = time;
            if (first) {
                fprintf(fptr,"%d[", ms);
                first = 0;
            }
            else fprintf(fptr,"*** %d|", ms);
        }

        // executa o processo
        Node *n = getLowestJob(ready);
        int index = getId(n) - 1;

        Process *p = (Process*)n->data;
        wait_time_ms[index] += ms - p->submission_time_ms;

        int throughput_ms = (*(int*) (p->cpu_bursts)->data);
        ms += throughput_ms; 
        fprintf(fptr,"P%d %d", p->id, ms);
        // tira o burst da lista
        dequeue(&p->cpu_bursts, 1, 2);

        Node *l = findLast(ready, n);

        if (p->cpu_bursts) { // se tiver outro burst, significad que tem pico de ES
            p->submission_time_ms = *(int*) p->cpu_bursts->data + ms; // tempo de resubmissao = ms + tempo de pico de ES
            if (is_seq && io_waiting) p->submission_time_ms += ((Process*)rear(io_waiting)->data)->submission_time_ms - ms;
            if (l)
                l->next = n->next;
            else if (ready) ready = ready->next;
            enqueue(&io_waiting, n);
            dequeue(&p->cpu_bursts, 1, 2);
        } else { // se nao tiver mais, dar dequeue pois acabou o processo
            if (l) l->next = n->next;
            else if (ready) ready = ready->next;
            freeNode(n, 1);
            proc_completed++;
            turnaround_time_ms[index] = ms - first_sbm_ms[index];
        }

    }
    float media_espera = 0.0f;
    float media_turnaround = 0.0f;
    fprintf(fptr,"]\n");
    fprintf(fptr,"[SJF] CPU %%: %.2f%%\n", 100 - 100 * ((float)inactive_ms/ms));
    fprintf(fptr,"[SJF] Throughtput: %.2f processos/segundo\n", proc_completed * (1000.0f / ms));
    for (int i = 0; i < list_size; i++) {
        fprintf(fptr,"[SJF] P%d Espera: %dms\n", i + 1, wait_time_ms[i]);
        fprintf(fptr,"[SJF] P%d Turnaround: %dms\n", i + 1, turnaround_time_ms[i]);
        media_espera += wait_time_ms[i];
        media_turnaround += turnaround_time_ms[i];
    }
    fprintf(fptr,"[SJF] MEDIA TEMPO DE ESPERA: %.2fms", media_espera / list_size);
    fprintf(fptr,"\n[SJF] MEDIA TEMPO DE TURNAROUND: %.2fms\n", media_turnaround / list_size);
    
}

void FCFS(Node* list, int is_seq, FILE* fptr) {
    Node *waiting = list;
    Node *io_waiting = NULL;
    Node *ready = NULL;
    int ms = 0;
    int first = 1;

    int list_size = getSize(list);

    int inactive_ms = 0;
    int proc_completed = 0;
    int first_sbm_ms[list_size];//alocar um array para guardar os primeiros tempos de submissao
    int wait_time_ms[list_size]; //alocar um array para guardar a espera total de cada processo
    int turnaround_time_ms[list_size]; //alocar um array para guardar a espera total de cada processo
    Node *ptr = waiting;
    for (int i = 0; i < list_size; i++) {
        first_sbm_ms[i] = ((Process*)ptr->data)->submission_time_ms;
        wait_time_ms[i] = 0;
        turnaround_time_ms[i] = 0;
        ptr = ptr->next;
    }

    fprintf(fptr,"[FCFS] GANTT: ");

    while (waiting || ready || io_waiting) { //enquanto tiver processos
        if (!first) fprintf(fptr,"|");
        
        checkReady(&waiting, &ready, ms);
        checkReady(&io_waiting, &ready, ms);
        

        // se nao tiver prontos, pega o primeiro da lista de espera e soma o tempo
        if (!ready) {
            int time = pullNextReady(&waiting, &io_waiting, &ready, is_seq);
            inactive_ms += time - ms;
            ms = time;
            if (first) {
                fprintf(fptr,"%d[", ms);
                first = 0;
            }
            else fprintf(fptr,"*** %d|", ms);
        }

        // executa o processo
        Process *p = (Process*) ready->data;
        int index = getId(ready) - 1;

        wait_time_ms[index] += ms - p->submission_time_ms;

        int throughput_ms = *(int*) (p->cpu_bursts)->data;
        ms += throughput_ms; 
        fprintf(fptr,"P%d %d", p->id, ms);
        // tira o burst da lista
        dequeue(&p->cpu_bursts, 1, 2);

        if (p->cpu_bursts) { // se tiver outro burst, significa que tem pico de ES
            p->submission_time_ms = (*(int*) front(p->cpu_bursts)->data) + ms; // tempo de resubmissao = ms + tempo de pico de ES
            if (is_seq && io_waiting) p->submission_time_ms += ((Process*)rear(io_waiting)->data)->submission_time_ms - ms;
            switchQueue(&ready, &io_waiting);
            dequeue(&p->cpu_bursts, 1, 2);
        } else { // se nao tiver mais, dar dequeue pois acabou o processo
            dequeue(&ready, 1, 1);
            proc_completed++;
            turnaround_time_ms[index] = ms - first_sbm_ms[index];
        }

    }
    float media_espera = 0.0f;
    float media_turnaround = 0.0f;
    fprintf(fptr, "]\n");
    fprintf(fptr, "[FCFS] CPU %%: %.2f%%\n", 100 - 100 * ((float)inactive_ms/ms));
    fprintf(fptr, "[FCFS] Throughtput: %.2f processos/segundo\n", proc_completed * (1000.0f / ms));
    for (int i = 0; i < list_size; i++) {
        fprintf(fptr, "[FCFS] P%d Espera: %dms\n", i + 1, wait_time_ms[i]);
        fprintf(fptr, "[FCFS] P%d Turnaround: %dms\n", i + 1, turnaround_time_ms[i]);
        media_espera += wait_time_ms[i];
        media_turnaround += turnaround_time_ms[i];
    }
    fprintf(fptr, "[FCFS] MEDIA TEMPO DE ESPERA: %.2fms", media_espera / list_size);
    fprintf(fptr, "\n[FCFS] MEDIA TEMPO DE TURNAROUND: %.2fms\n", media_turnaround / list_size);
}


int main(int argc, char **argv) {

    if (argc < 3 || argc >= 5) {
        printf("argumentos insuficientes");
        exit(EXIT_FAILURE);
    }

    char *filename = argv[1];
    int quantum = atoi(argv[2]);
    int isSeq = 0;
    if (argc == 4) isSeq = ! strcmp(argv[3], "-seq");
    
    //ler arquivo
    FILE *file_ptr;
    if ((file_ptr = fopen(filename, "r")) == NULL) {
        printf("Erro ao abrir arquivo");
        exit(EXIT_FAILURE);
    }

    char buffer[128];

    Node *process_root = NULL;
    unsigned int process_id = 1;

    while (fgets(buffer, 128, file_ptr) != NULL) {
        char *token;
        token = strtok(buffer, " ");

        int data[100];
        int data_size = 0;

        while (token != NULL) {
            data[data_size++] = atoi(token);
            token = strtok(NULL, " ");
        }

        if(data_size < 3) continue;
        Process p;
        p.priority = data[0];
        p.submission_time_ms = data[1];
        p.cpu_bursts = NULL;
        p.id = process_id++;

        for(int i = 2; i < data_size; i++)  
            enqueue(&p.cpu_bursts, createNodeBurst(data[i]));
    
        enqueue(&process_root, createNodeProc(p));

    }
    fclose(file_ptr);

    strcat(filename, ".out");
    if ((file_ptr = fopen(filename, "w")) == NULL) {
        printf("Erro ao abrir arquivo");
        exit(EXIT_FAILURE);
    }

    Node *temp = createCopy(process_root);
    FCFS(temp, isSeq, file_ptr);

    fprintf(file_ptr, "\n");

    temp = createCopy(process_root);
    SJF(temp, isSeq, file_ptr);

    fprintf(file_ptr, "\n");

    temp = createCopy(process_root);
    SRTF(temp, isSeq, file_ptr);

    fprintf(file_ptr, "\n");

    temp = createCopy(process_root);
    PP(temp, isSeq, file_ptr);

    fprintf(file_ptr, "\n");

    temp = createCopy(process_root);
    RR(temp, quantum, isSeq, file_ptr);

    fclose(file_ptr);

}