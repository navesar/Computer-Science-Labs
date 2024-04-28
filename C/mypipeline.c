/* Team members: Nave Sarfati: 315212753, Yuval Shulman: 208825760 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

void exitAbnormally(char* funcName){
    fprintf(stderr, "Error %s()\n", funcName);
    exit(1);
}


int main(void)
{
    int read_write[2], status1, status2;
    pid_t pid1, pid2;
    char* ls_l[] = {"ls", "-l", NULL};
    char* tail_n2[] ={"tail", "-n", "2", NULL};
    if (pipe(read_write) == -1) 
        exitAbnormally("pipe");
    
    fprintf(stderr, "(parent_process>forking…)\n");
    pid1 = fork();
    fprintf(stderr, "(parent_process>created process with id: %d)\n",pid1);

    if (pid1 == -1) 
        exitAbnormally("fork");

    if (pid1 == 0) {  // child process
        fprintf(stderr,"(child1>redirecting stdout to the write end of the pipe…)\n");
        if(dup2(read_write[1], STDOUT_FILENO) == -1)
            exitAbnormally("dup2");

        if(close(read_write[1]) == -1)
            exitAbnormally("close");
        fprintf(stderr, "(child1>going to execute cmd: ls -l)\n");
        if(execvp(ls_l[0], ls_l) == -1)
            exitAbnormally("execvp");

        exit(0);
    } else {  // parent process
        fprintf(stderr, "(parent_process>closing the write end of the pipe…)\n");
        if(close(read_write[1]) == -1)
            exitAbnormally("close");
        pid2 = fork();

        if (pid2 == -1) 
            exitAbnormally("fork");

        if (pid2 == 0) {  // child process
            fprintf(stderr,"(child2>redirecting stdin to the read end of the pipe…)\n");
            if(dup2(read_write[0], STDIN_FILENO) == -1)
                exitAbnormally("dup2");

            if(close(read_write[0]) == -1)
                exitAbnormally("close");
            fprintf(stderr, "(child2>going to execute cmd: tail -n 2)\n");
            if(execvp(tail_n2[0], tail_n2) == -1)
                exitAbnormally("execvp");

            exit(0);
        }
        else{
            fprintf(stderr, "(parent_process>closing the read end of the pipe…)\n");
            if(close(read_write[0]) == -1)
                exitAbnormally("close");
            fprintf(stderr, "(parent_process>waiting for child processes to terminate…)\n");
            if(waitpid(pid1, &status1, 0) == -1)
                exitAbnormally("waitpid");
            fprintf(stderr, "(parent_process>waiting for child processes to terminate…)\n");
            if(waitpid(pid2, &status2, 0) == -1)
                exitAbnormally("waitpid");
            fprintf(stderr,"(parent_process>exiting…)\n");
            exit(0);
        }
    }
}
