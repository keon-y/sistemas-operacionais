//AUTOR: VICTOR KEONY OKABAYASHI


#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <dirent.h>
#include <ctype.h>

#define MAX_ARGS 15
#define CMD_MAX_LENGTH 50

//cores
#define RED   "\x1B[31m"
#define GRN   "\x1B[32m"
#define YEL   "\x1B[33m"
#define BLU   "\x1B[34m"
#define MAG   "\x1B[35m"
#define CYN   "\x1B[36m"
#define WHT   "\x1B[37m"
#define RESET "\x1B[0m"
#define BOLD "\e[1m"
#define RBOLD "\e[m"

typedef struct Node Node;
Node* createNode(int, char*);
Node* findNode(Node*, int); 
void freeTree(Node*);
void printTree(Node*, int);
void addChild(Node*, Node*);
void buildTree(int, Node**);
char* replaceWord(char*, char*, char*);

int main () {

    printf("\n= = = = = PROGRAMA" GRN BOLD " INICIADO" RESET RBOLD" = = = = =\n");

    char dir[50];
    char dir2[50];

    char *arg = malloc(sizeof(char) * CMD_MAX_LENGTH);
    char *argv[CMD_MAX_LENGTH];
    int argc;
    char currDir[100];

    int shouldWait;
    
    while (1) {

        //por padrao esperar
        shouldWait = 1;

        //pegar o diretorio atual
        getcwd(currDir, 100);

        char to_replace[30];
        strcpy(to_replace, "/home/");
        strcat(to_replace, getlogin());

        strcpy(currDir, replaceWord(currDir, to_replace, "~"));
        

        //ler o input
        char host[30];
        gethostname(host, 30);
        printf(BOLD GRN "\n%s@%s" RESET ":" BOLD BLU "%s" RESET RBOLD "$ " , getlogin(),host,  currDir);

        argc = 0;

        strcpy(dir, "/usr/bin/");
        strcpy(dir2, "/snap/bin/");

        fgets(arg, CMD_MAX_LENGTH, stdin);

        //tratamento
        arg[strcspn(arg, "\n")] = '\0'; //tirar o \n que vem com o enter
        while (*arg == ' ') arg++;
        if (arg[0] == '\0') {
            printf(RED BOLD "LINHA DE COMANDO VAZIA !" RESET );
            continue;
        }


        //tratar a string, separar por espacos
        argv[argc++] = strtok(arg, " ");
        while ((argv[argc++] = strtok(NULL, " ")) != NULL);
        argv[argc--] = NULL;

        //nao escreveu nada ou so escreveu &
        if (argc == 0 || (argc == 1 && strcmp(argv[0], "&") == 0)) {
            printf(RED BOLD "LINHA DE COMANDO VAZIA !" RESET );
            continue;
        }

        //testar o &
        if (strcmp(argv[argc - 1], "&") == 0) {
            argv[--argc] = NULL;
            shouldWait = 0; //nao vai esperra
        }

        strcat(dir, argv[0]);
        strcat(dir2, argv[0]);

        if (strcmp(argv[0], "exit") == 0) 
            break;
        

        else if(strcmp(argv[0], "tree") == 0) {

            if(argc != 2) {
                printf(BOLD BLU "USO" RESET ":" RBOLD " tree {PID}");
                continue;
            }

            Node *root = NULL;
            buildTree(atoi(argv[1]), &root);
            if (!root) 
                 printf(RED BOLD "PROCESSO INEXISTENTE OU NAO ENCONTRADO !" RESET RBOLD);
            else
                printTree(root, 0);

            freeTree(root);
            
        }

        else if (strcmp(argv[0], "cd") == 0)  //cd
            chdir(argv[1]);
    
        else {
            if (fork() == 0) {
                if (execve(dir, argv, __environ) == -1 && execve(dir2, argv, __environ) == -1) 
                    printf(RED BOLD "COMANDO NAO ENCONTRADO !" RESET );
                
            }
            else {
                if (shouldWait)
                    wait(NULL);
            }
            
        }
    }

     printf("\n= = = = = PROGRAMA" RED BOLD " ENCERRADO" RESET RBOLD" = = = = =\n");
    free(arg);


    return 0;
}

typedef struct Node {

    char p_name[30]; //nome do processo
    int pid; //pid
    int num_child; //quantidade de filhos
 
    struct Node **children; //filhos
   
} Node;

// Funcao para criar um no com PID pid e NOME name
Node* createNode(int pid, char *name) {
    Node *new_node = (Node*)malloc(sizeof(Node));
    strcpy(new_node->p_name, name);
    new_node->pid = pid;
    new_node->num_child = 0;
    new_node->children = NULL;
    return new_node;
}

//busca um no recursivamente pelo PID partindo de n
Node* findNode(Node* n, int pid) {

    if (!n) return NULL;
    if (n->pid == pid) return n;

    Node *temp;

    for (int i = 0; i < n->num_child; i++) {
        temp = findNode(n->children[i], pid);
        if (temp != NULL)
            return temp;
    }
    return NULL;
}

//desalocar arvore
void freeTree(Node* node) {
    if(!node) return;
    if(node->num_child == 0) {
        free(node);
        return;
    }

    for(int i = 0; i < node->num_child; i++)
        freeTree(node->children[i]);

}

//printa a arvore recursivamente
void printTree(Node *node, int level) {
    if (!node) return;
    for (int i = 0; i < level - 1; i++) printf("   ");
    if(level != 0) printf("`-");
    printf("%d %s\n", node->pid, node->p_name);

    for(int i = 0; i < node->num_child; i++) 
        printTree(node->children[i], level + 1);
    
}

// Funcao para adicionar um no child para um no parent
void addChild(Node *parent, Node *child) {
    parent->children = (Node**)realloc(parent->children, sizeof(Node*) * (parent->num_child + 1));
    parent->children[parent->num_child++] = child;
}

void buildTree(int pid, Node **root){

    DIR *dir; // /proc
    struct dirent *entry;
    char path[256];
    int is_number;

    if ((dir = opendir("/proc")) == NULL) {
        perror("Erro ao abrir diretorio");
        return;
    }
    
    while ((entry = readdir(dir)) != NULL) {
        
        //filtrar pastas e arquivos que nao sao processos
        is_number = 1;
        for (int i = 0; i < strlen(entry->d_name); i++) {
            if (!isdigit(entry->d_name[0])) {
                is_number = 0;
                break;
            }
        }
        if (!is_number) continue;


        //abrir a pasta e checar o arquivo stat
        strcpy(path, "/proc/");
        strcat(path, entry->d_name);
        strcat(path, "/stat");
        FILE *file = fopen(path, "r");
        if (file == NULL) {
            printf("Erro ao abrir arquivo %s\n", path);
            exit(EXIT_FAILURE);
        }
            
        int proc_pid;
        char proc_name[256];
        char proc_status;
        int proc_ppid;

        fscanf(file, "%d %s %c %d", &proc_pid, proc_name, &proc_status, &proc_ppid);
        fclose(file);


        if (atoi(entry->d_name) == pid) *root = createNode(pid, proc_name); //processo raiz
        else if (atoi(entry->d_name) > pid) {//potencial filho
            Node *parent = findNode(*root, proc_ppid);                    //se encontra o processo pai na arvore
            if(parent) addChild(parent, createNode(proc_pid, proc_name)); //adicionar ele como filho
        } 
        
    }
    closedir(dir);
}

//troca uma substring old por uma substring new em uma string s
char* replaceWord(char* s, char* old, char* new) 
{ 
    char* result; 
    int cnt = 0; 
    int i;
    size_t new_len = strlen(new); 
    size_t old_len = strlen(old); 
 
    for (i = 0; s[i] != '\0'; i++) { 
        if (strstr(&s[i], old) == &s[i]) { 
            cnt++; 
            i += old_len - 1; 
        } 
    } 
 
    result = (char*)malloc(i + cnt * (new_len - old_len) + 1); 
 
    i = 0; 
    while (*s) { 
        if (strstr(s, old) == s) { 
            strcpy(&result[i], new); 
            i += new_len; 
            s += old_len; 
        } 
        else
            result[i++] = *s++; 
    } 
 
    result[i] = '\0'; 
    return result; 
} 