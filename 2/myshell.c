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

void freeCmdLines2(cmdLine *pCmdLine)
{
    if(pCmdLine != NULL){
        freeCmdLines(pCmdLine);
        pCmdLine = NULL;
    }
}
void printCmd(cmdLine *pCmdLine)
{
    printf("cmdLine:");
    for(int i = 0; i < pCmdLine->argCount; i++){
        printf("%s ", pCmdLine->arguments[i]);
    }
    printf("\n");
}

void execute(cmdLine *pCmadLine, int debug)
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
            redirect = open(pCmadLine->inputRedirect, O_RDONLY);
            if(redirect == -1){
                perror("Failed to open input file\n");
                freeCmdLines2(pCmadLine);
                _exit(1);
            }
            dup2(redirect, STDIN_FILENO);
            close(redirect);
        }
        if(pCmadLine->outputRedirect != NULL){
            redirect = open(pCmadLine->outputRedirect, O_CREAT|O_WRONLY|O_TRUNC, S_IRWXU);
            if(redirect == -1){
                perror("Failed to open output file\n");
                freeCmdLines2(pCmadLine);
                _exit(1);
            }
            dup2(redirect, STDOUT_FILENO);
            close(redirect);
        }
        if(execvp(pCmadLine->arguments[0], pCmadLine->arguments) == -1){
            perror("execv failed\n");
            freeCmdLines2(pCmadLine);
            _exit(1);
        }
    }
    else{
        if(pCmadLine->blocking){
            if(waitpid(pid, &status, 0) == -1){
                perror("waitpid failed\n");
                exit(1);
            }
        }
        freeCmdLines2(pCmadLine);
    }
}

int main(int argc, char *argv[])
{
    char path[PATH_MAX];
    char userInput[2048];
    cmdLine *parsed;
    int debug = 0;
    if(argc > 1 && strcmp(argv[1], "-d") == 0)
        debug = 1;
    while (1)
    {
        if(getcwd(path, sizeof(path)) != NULL)
            printf("Current working directory: %s>", path);
        else
            printf("getcwd() error!\n");
        if(fgets(userInput, 2048, stdin) == NULL){
            break;
        }
        if(strcmp(userInput, "quit\n") == 0 || strcmp(userInput, "quit") == 0){
            exit(0);
        }
        else if (strncmp(userInput, "suspend ", 8) == 0)
        {
            pid_t pid = atoi(userInput + 8);
            if(kill(pid, SIGTSTP) == -1)
                perror("kill failed\n");
            else
                printf("Proccess %d suspended\n", pid);
        }
        else if (strncmp(userInput, "wake ", 5) == 0)
        {
            pid_t pid = atoi(userInput + 5);
            if(kill(pid, SIGCONT) == -1)
                perror("kill failed\n");
            else
                printf("Proccess %d woke up\n", pid);
        }
        else if (strncmp(userInput, "kill ", 5) == 0)
        {
            pid_t pid = atoi(userInput + 5);
            if(kill(pid, SIGKILL) == -1)
                perror("kill failed\n");
            else
                printf("Proccess %d terminated\n", pid);
        }
        else{ 
            parsed = parseCmdLines(userInput);
            execute(parsed, debug);
        }
    }
    return 0;
}
