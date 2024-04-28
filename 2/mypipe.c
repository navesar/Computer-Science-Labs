/* Team members: Nave Sarfati: 315212753, Yuval Shulman: 208825760 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>



int main(void)
{
    int read_write[2];
    pid_t pid;
    char message[2048];
    if (pipe(read_write) == -1) {
        perror("Error in pipe()\n");
        exit(1);
    }
    pid = fork();
    if (pid == -1) {
        perror("Error fork()\n");
        exit(1);
    }

    if (pid == 0) {  // child process
        close(read_write[0]);

        strncpy(message, "100 please", 2048);
        write(read_write[1], message, 2048);

        close(read_write[1]);

        exit(0);
    } else {  // parent process
        close(read_write[1]);

        read(read_write[0], message, 2048);
        printf("Received message from child: %s\n", message);

        close(read_write[0]);

        exit(0);
    }
}
