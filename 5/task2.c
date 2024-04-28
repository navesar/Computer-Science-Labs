#include <stdio.h>
#include <unistd.h>
#include <elf.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>

extern int startup(int argc, char **argv, void (*start)());

char* getType(int type){
    return type == PT_HIPROC ? "HIPROC" :
           type == PT_LOPROC ? "LOPROC" :
           type == PT_HIOS ? "HIOS" :
           type == PT_SUNWSTACK ? "SUNWSTACK" :
           type == PT_SUNWBSS ? "SUNWBSS" :
           type == PT_GNU_RELRO ? "GNU_RELRO" :
           type == PT_GNU_STACK ? "GNU_STACK" :
           type == PT_GNU_EH_FRAME ? "GNU_EH_FRAME" :
           type == PT_NUM ? "NUM" :
           type == PT_TLS ? "TLS" :
           type == PT_PHDR ? "PHDR" :
           type == PT_SHLIB ? "SHLIB" :
           type == PT_NOTE ? "NOTE" :
           type == PT_INTERP ? "INTERP" :
           type == PT_DYNAMIC ? "DYNAMIC" :
           type == PT_LOAD ? "LOAD" :
           type == PT_NULL ? "NULL" :
           "Unknown";    
}


char* getFlag(int flag){
    return flag == 4  ? "R" :
           flag == 5 ? "R E" :
           flag == 6 ? "RW" :
           "Unknown";
}

int getFlag2(int flg){
    switch (flg){
        case 0x000: return 0;
        case 0x001: return PROT_EXEC;
        case 0x002: return PROT_WRITE;
        case 0x003: return PROT_EXEC | PROT_EXEC;
        case 0x004: return PROT_READ;
        case 0x005: return PROT_READ | PROT_EXEC;
        case 0x006: return PROT_READ | PROT_WRITE;
        case 0x007: return PROT_READ | PROT_WRITE | PROT_EXEC;
        default: return -1;
    }
}


void print_phdr_info(Elf32_Phdr *phdr,int index){
    if(phdr->p_type == PT_LOAD){
        printf("%s\t\t%#08x\t%#08x\t%#10.08x\t%#07x\t%#07x\t%s\t%#-6.01x",
            getType(phdr->p_type), phdr->p_offset, phdr->p_vaddr, phdr->p_paddr,
            phdr->p_filesz, phdr->p_memsz, getFlag(phdr->p_flags), phdr->p_align);  
        int convertedFlag = getFlag2(phdr->p_flags);
        printf("   flag : %d", convertedFlag);
        printf("\n");
    }
    else{
        printf("%s\t\t%#08x\t%#08x\t%#10.08x\t%#07x\t%#07x\t%s\t%#-6.01x\n",
                getType(phdr->p_type),phdr->p_offset,phdr->p_vaddr,phdr->p_paddr,phdr->p_filesz,phdr->p_memsz,getFlag(phdr->p_flags),phdr->p_align);
    }
}



void load_phdr(Elf32_Phdr *phdr, int fd){
    if(phdr->p_type == PT_LOAD){
        void *vadd = (void *)(phdr -> p_vaddr & 0xfffff000);
        int offset = phdr->p_offset & 0xfffff000;
        int padding = phdr->p_vaddr & 0xfff;
        int convertedFlag = getFlag2(phdr->p_flags);
        if ((mmap(vadd, phdr->p_memsz+padding, convertedFlag, MAP_FIXED | MAP_PRIVATE, fd, offset)) == MAP_FAILED){
            perror("mmap failed in load_phdr");
            exit(1);
        }
        print_phdr_info(phdr, 0);
    }
}

int foreach_phdr(void *map_start, void (*func)(Elf32_Phdr *,int), int arg){
    Elf32_Ehdr* header = (Elf32_Ehdr*)map_start;
    for (size_t i = 0; i < header->e_phnum; i++)    
        func(map_start + header->e_phoff + (i * header->e_phentsize), arg);
}

int main(int argc, char** argv){
    if(argc < 2){
        fprintf(stderr, "Please provide an argument to loader\n");
        return 1;
    }
    int fd = open(argv[1], O_RDWR);
    if(fd < 0){
        fprintf(stderr, "Error in open %s\n", argv[1]);
        return 1;
    }
    struct stat fd_stat;
    if(fstat(fd, &fd_stat) != 0 ) {
      fprintf(stderr, "Error: stat fail\n");
      return 1;
    }
    void* map_start = mmap(0, fd_stat.st_size, PROT_READ | PROT_WRITE , MAP_SHARED, fd, 0);
    if(map_start == MAP_FAILED){
        fprintf(stderr, "Error: mmap fail\n");
        return 1;
    }
    Elf32_Ehdr* header = (Elf32_Ehdr *) map_start;
    foreach_phdr(map_start, load_phdr, fd);
    startup(argc - 1, argv + 1, (void *)(header->e_entry));
    close(fd);
    return 0;
}