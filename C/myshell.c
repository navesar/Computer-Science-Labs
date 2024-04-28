/* Team members: Nave Sarfati: 315212753, Yuval Shulman: 208825760 */
#include<stdio.h>
#include<string.h>
#include"LineParser.h"
#include<unistd.h>
#include<linux/limits.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<signal.h>

typedef struct process{
        cmdLine* cmd;                         /* the parsed command line*/
        pid_t pid; 		                  /* the process id that is running the command*/
        int status;                           /* status of the process: RUNNING/SUSPENDED/TERMINATED */
        struct process *next;	                  /* next process in chain */
    } process;
#define TERMINATED -1
#define RUNNING 1
#define SUSPENDED 0
#define HISTLEN 20
void freeProcessList(process** process_list);
void printCmd(cmdLine *pCmdLine);
void updateProcessStatus(process* process_list, int pid, int status);

void addProcess(process** process_list, cmdLine* cmd, pid_t pid){
    process* p = (process*)malloc(sizeof(process));
    p->cmd = cmd;
    p->pid = pid;
    p->status = RUNNING;
    p->next = *process_list;
    *process_list = p;
    
}
void printCmd(cmdLine *pCmdLine)
{
    for(int i = 0; i < pCmdLine->argCount; i++){
        printf("%s ", pCmdLine->arguments[i]);
    }
    printf("\n");
}
void freeCmdLines2(cmdLine *pCmdLine)
{
    if(pCmdLine != NULL){
        freeCmdLines(pCmdLine);
        pCmdLine = NULL;
    }
}
void updateProcessList(process **process_list){
    if(process_list == NULL){
        printf("process_list == NULL!\n");
        return;
    }
    int status;
    pid_t pid;
    process* curr = *process_list;
    if (curr == NULL)
    {
        printf("EMPTY LIST!\n");
        return;
    }
    
    while(curr != NULL){
        pid = curr->pid;
        status = 0;
        if(waitpid(pid, &status, WNOHANG) == -1 || WIFSIGNALED(status)){
            updateProcessStatus(curr, pid, TERMINATED);
        }
        else if (WIFSTOPPED(status)) {
            updateProcessStatus(curr, pid, SUSPENDED);
        }
        else if (WIFCONTINUED(status)) {
            updateProcessStatus(curr, pid, RUNNING);
        }
        curr = curr->next;
    }
}
void printProcessList(process** process_list){
    updateProcessList(process_list);
    int index = 0;
    process *curr = *process_list;
    process *prev = NULL;
    process *next = NULL;
    while(curr != NULL){
        printf("%d %d %d ", index++, curr->pid, curr->status);
        if(curr->status == TERMINATED){
            freeCmdLines2(curr->cmd);
            if(prev == NULL)
                *process_list = curr->next;
            else
                prev->next = curr->next;
            next = curr->next;
            free(curr);
            curr = next;    
        }
        else{
            prev = curr;
            curr = curr->next;
        }
    }
        
}


void freeProcessList(process** process_list){
    process* curr = *process_list;
    process* next;
    while(curr != NULL){
        next = curr->next;
        freeCmdLines2(curr->cmd);
        free(curr);
        curr = next;
    }
    *process_list = NULL;
    free(process_list);
}

void updateProcessStatus(process* process_list, int pid, int status){
    process *curr = process_list;
    while(curr != NULL){
        if(curr->pid == pid){
            curr->status = status;
            return;
        }
        curr = curr->next;
    }
    fprintf(stderr ,"Process with the pid: %d not found\n", pid);
}

void execute(cmdLine *pCmadLine, int debug, int index, int f, pid_t original, process** process_list)
{
    int status, redirect;
    if(strcmp(pCmadLine->arguments[0], "cd") == 0){
        if(debug){
            fprintf(stderr, "PID: %d\n",0);
            fprintf(stderr, "Executing command: %s\n", pCmadLine->arguments[0]);
        }
        if(chdir(pCmadLine->arguments[1]) == -1){
            fprintf(stderr, "cd fail\n");
        }
        freeCmdLines2(pCmadLine);
        return;
    }
    if(f){
        pid_t pid = fork();
        if(pid == -1){
            perror("fork failed\n");
            freeCmdLines2(pCmadLine);
            exit(1);
        }
        if(pid == 0){
            if(debug){
            fprintf(stderr, "PID: %d\n",pid);
            fprintf(stderr, "Executing command: %s\n", pCmadLine->arguments[0]);
            }
            if(pCmadLine->inputRedirect != NULL){
                if(index == 1){
                    perror("Cannot redirect the input of the right process\n");
                    exit(1);
                }
                redirect = open(pCmadLine->inputRedirect, O_RDONLY);
                if(redirect == -1){
                    perror("Failed to open input file\n");
                    _exit(1);
                }
                dup2(redirect, STDIN_FILENO);
                close(redirect);
            }
            if(pCmadLine->outputRedirect != NULL){
                if(index == 0){
                    perror("Cannot redirect the output of the left process\n");
                    exit(1);
                }
                redirect = open(pCmadLine->outputRedirect, O_CREAT|O_WRONLY|O_TRUNC, S_IRWXU);
                if(redirect == -1){
                    perror("Failed to open output file\n");
                    _exit(1);
                }
                dup2(redirect, STDOUT_FILENO);
                close(redirect);
            }
            if(execvp(pCmadLine->arguments[0], pCmadLine->arguments) == -1){
                perror("execvp failed");
                _exit(1);
            }
            // else
            //     addProcess(process_list, pCmadLine, pid);
        }
        else{
            addProcess(process_list, pCmadLine, pid);
            if(pCmadLine->blocking){
                if(waitpid(pid, &status, 0) == -1){
                    perror("waitpid failed\n");
                    exit(1);
                }
            }
        }
    }
    else{
        if(debug){
        fprintf(stderr, "PID: %d\n",original);
        fprintf(stderr, "Executing command: %s\n", pCmadLine->arguments[0]);
        }
        if(pCmadLine->inputRedirect != NULL){
            if(index == 1){
                perror("Cannot redirect the input of the right process\n");
                exit(1);
            }
            redirect = open(pCmadLine->inputRedirect, O_RDONLY);
            if(redirect == -1){
                perror("Failed to open input file\n");
                _exit(1);
            }
            dup2(redirect, STDIN_FILENO);
            close(redirect);
        }
        if(pCmadLine->outputRedirect != NULL){
            if(index == 0){
                perror("Cannot redirect the output of the left process\n");
                exit(1);
            }
            redirect = open(pCmadLine->outputRedirect, O_CREAT|O_WRONLY|O_TRUNC, S_IRWXU);
            if(redirect == -1){
                perror("Failed to open output file\n");
                _exit(1);
            }
            dup2(redirect, STDOUT_FILENO);
            close(redirect);
        }
        if(execvp(pCmadLine->arguments[0], pCmadLine->arguments) == -1){
            perror("execvp failed");
            _exit(1);
        }
    }
}
void exePipe(char *userInput, int debug, process** process_list){
    int read_write[2], status1, status2;
    pid_t pid1, pid2;
    char* cpy = (char*)malloc(strlen(userInput)+1);
    strcpy(cpy,userInput);
    char* cmd1 = strtok(cpy, "|");
    char* cmd2 = strtok(NULL, "|");
    if(cmd1 == NULL || cmd2 == NULL || pipe(read_write) == -1 || (pid1 = fork()) == -1){
        fprintf(stderr, "Invalid pipe cmd!\n");
        free(cpy);
        return;
    }
    if(pid1 == 0){
        if(dup2(read_write[1], STDOUT_FILENO) == -1 || close(read_write[1]) == -1){
            printf("error in process1...\n");
            exit(1);
        }
        execute(parseCmdLines(cmd1), debug, 0, 0, pid1, process_list); //left side process 
        exit(0);
    }
    else{
        addProcess(process_list, parseCmdLines(cmd1), pid1);
        if(close(read_write[1]) == -1 || (pid2 = fork()) == -1){
            if(waitpid(pid1, &status1, 0) == -1)
                exit(1);
            exit(1);
        }
        if(pid2 == 0){
            if(dup2(read_write[0], STDIN_FILENO) == -1 || close(read_write[0]) == -1)
                exit(1);
            execute(parseCmdLines(cmd2), debug, 1, 0, pid2, process_list); //right side process
            exit(0);
        }
        else{
            addProcess(process_list, parseCmdLines(cmd2), pid2);
            if(close(read_write[0]) == -1 || waitpid(pid1, &status1, 0) == -1 || waitpid(pid2, &status2, 0) == -1)
                perror("close / waitpid error\n");
        }
    }
    free(cpy);
    
}

int main(int argc, char *argv[])
{
    process** process_list = (process**)malloc(sizeof(process**));
    *process_list = NULL;
    char path[PATH_MAX];
    char userInput[2048];
    cmdLine *parsed;
    int debug = 0;
    char* history[HISTLEN];
    int newest = 0;
    if(argc > 1 && strcmp(argv[1], "-d") == 0)
        debug = 1;
    while (1)
    {
        if(getcwd(path, sizeof(path)) != NULL)
            printf("Current working directory: %s$ ", path);
        else
            printf("getcwd() error!\n");
        if(fgets(userInput, 2048, stdin) == NULL){
            printf("\n");
            break;
        }
        if(strcmp(userInput, "quit\n") == 0 || strcmp(userInput, "quit") == 0){
            exit(0);
        }
        else if (strcmp(userInput, "\n") == 0)
        {
            //do nothing
        }
        
        else if (strchr(userInput, '|') != NULL)
        {
            exePipe(userInput, debug, process_list);
        }
        else if (strcmp(userInput, "procs\n") == 0 || strcmp(userInput, "procs") == 0)
        {
            printProcessList(process_list);
        }
        else if (strncmp(userInput, "suspend ", 8) == 0)
        {
            pid_t pid = atoi(userInput + 8);
            if(kill(pid, SIGTSTP) == -1)
                perror("kill failed\n");
            else{
                updateProcessStatus(*process_list, pid, SUSPENDED);
                printf("Process %d suspended\n", pid);
            }
        }
        else if (strncmp(userInput, "wake ", 5) == 0)
        {
            pid_t pid = atoi(userInput + 5);
            if(kill(pid, SIGCONT) == -1)
                perror("kill failed\n");
            else{
                updateProcessStatus(*process_list, pid, RUNNING);
                printf("Process %d woke up\n", pid);
            }
        }
        else if (strncmp(userInput, "kill ", 5) == 0)
        {
            pid_t pid = atoi(userInput + 5);
            if(kill(pid, SIGKILL) == -1)
                perror("kill failed\n");
            else{
                updateProcessStatus(*process_list, pid, TERMINATED);
                printf("Process %d terminated\n", pid);
            }
        }
        else if(strcmp(userInput, "history\n") != 0 && userInput[0] != '!'){ 
            parsed = parseCmdLines(userInput);
            execute(parsed, debug, -1, 1, -1, process_list); //-1 = parent process
        }


        if(strcmp(userInput, "\n") != 0 && userInput[0] != '!'){
            if(newest == HISTLEN){ //history is full
                free(history[0]);
                for(int i = 0; i < HISTLEN - 1; i++){
                    history[i] = history[i+1];
                }
                history[HISTLEN - 1] = (char*)malloc(sizeof(userInput));
                strcpy(history[HISTLEN - 1], userInput);
            }
            else{
                history[newest] = (char*)malloc(sizeof(userInput));
                strcpy(history[newest++], userInput);
            }
        }
        if(strcmp(userInput, "history\n") == 0){
            for(int i = 0; i < newest; i++)
                printf("%s", history[i]);
        }
        else if (userInput[0] == '!')
        {
            int legal = 1;
            if(userInput[1] == '!') {   
                if(newest == 0){
                    legal = 0;
                    fprintf(stderr, "history is empty\n");
                }
                else
                    strcpy(userInput, history[newest-1]);
            }
            else{
                int ind = strtol(userInput+1, NULL, 10);
                if(ind > newest -1){
                    legal = 0;
                   fprintf(stderr, "invalid index\n"); 
                }
                else
                    strcpy(userInput, history[ind]);
            } 
            if(legal){    
                if (strchr(userInput, '|') != NULL)
                {
                    exePipe(userInput, debug, process_list);
                }
                else if (strcmp(userInput, "procs\n") == 0 || strcmp(userInput, "procs") == 0)
                {
                    printProcessList(process_list);
                }
                else if (strncmp(userInput, "suspend ", 8) == 0)
                {
                    pid_t pid = atoi(userInput + 8);
                    if(kill(pid, SIGTSTP) == -1)
                        perror("kill failed\n");
                    else{
                        updateProcessStatus(*process_list, pid, SUSPENDED);
                        printf("Process %d suspended\n", pid);
                    }
                }
                else if (strncmp(userInput, "wake ", 5) == 0)
                {
                    pid_t pid = atoi(userInput + 5);
                    if(kill(pid, SIGCONT) == -1)
                        perror("kill failed\n");
                    else{
                        updateProcessStatus(*process_list, pid, RUNNING);
                        printf("Process %d woke up\n", pid);
                    }
                }
                else if (strncmp(userInput, "kill ", 5) == 0)
                {
                    pid_t pid = atoi(userInput + 5);
                    if(kill(pid, SIGKILL) == -1)
                        perror("kill failed\n");
                    else{
                        updateProcessStatus(*process_list, pid, TERMINATED);
                        printf("Process %d terminated\n", pid);
                    }
                }
                else if(strcmp(userInput, "history\n") != 0){ 
                    parsed = parseCmdLines(userInput);
                    execute(parsed, debug, -1, 1, -1, process_list); //-1 = parent process
                }
                if(strcmp(userInput, "\n") != 0 && strchr(userInput,'!') == NULL){
                    if(newest == HISTLEN){ //history is full
                        free(history[0]);
                        for(int i = 0; i < HISTLEN - 1; i++){
                            history[i] = history[i+1];
                        }
                        history[HISTLEN - 1] = (char*)malloc(sizeof(userInput));
                        strcpy(history[HISTLEN - 1], userInput);
                    }
                    else{
                        history[newest] = (char*)malloc(sizeof(userInput));
                        strcpy(history[newest++], userInput);
                    }
                    if(strcmp(userInput, "history\n") == 0){
                        for(int i = 0; i < newest; i++)
                            printf("%s", history[i]);
                    }
                }
            }
        }
            }
            
            
                
            
    for(int i = 0; i < newest; i++)
        free(history[i]);
    freeProcessList(process_list);
    exit(0);
}
