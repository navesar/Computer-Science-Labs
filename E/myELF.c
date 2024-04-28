#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <elf.h>
#include<string.h>

int debug = 0;
void* files_data[128];
int files_sizes[128];
char files_names[128][256];
int fds[128];
int myIndex = 0;


// struct fun_desc
//     {
//         char *name;
//         void (*fun)(state*);
//     };
struct fun_desc
    {
        char *name;
        void (*fun)();
    };
void toggleDebugMode();    
void examine();
void printSectionNames();
void printsyms();
void CheckMerge();
void mergeFiles();
void quit();
struct fun_desc menu[] = {{"Toggle Debug Mode", toggleDebugMode},
                            {"Examine ELF File", examine},
                            {"Print Section Names", printSectionNames},
                            {"Print syms", printsyms},
                            {"Check Files for Merge", CheckMerge},
                            {"Merge ELF Files", mergeFiles},
                            {"Quit", quit},
                            {NULL, NULL}};

int main(int argc, char const *argv[])
{
    char userInput[2048];
    while(1){
        for(int i = 0; menu[i].name != NULL; i++)
                printf("%d-%s\n", i, menu[i].name);
        if(fgets(userInput, 2048, stdin) == NULL)
            quit();
        else if(userInput[1] != '\n' || userInput[0] - '0' < 0 || userInput[0] - '0' > 8)
            printf("Not within bounds (%s)", userInput);
        else
            menu[userInput[0] - '0'].fun();
    }
    return 0;
}


void toggleDebugMode(){
    printf("Debug mode is now %s\n", debug ? "off" : "on");
    debug = !debug;
}

void quit(){
    for(int i = 0; i < myIndex; i++){
        if(fds[i] != -1){
            munmap(files_data[i], files_sizes[i]);
            close(fds[i]);
        }
    }
    exit(0);
}

void examine(){ //handles only 1 file at a time
    char file_name[256];
    printf("Enter the ELF file name: ");
    if (fgets(file_name, sizeof(file_name), stdin) == NULL) {
        perror("Error reading file name");
        return;
    }
    file_name[strcspn(file_name, "\n")] = '\0';
    strcpy(files_names[myIndex], file_name);
     // Open the file for reading
    int fd = open(file_name, O_RDONLY);
    if (fd == -1) {
        perror("Error opening file");
        return;
    }

    // Get the size of the file
    off_t file_size = lseek(fd, 0, SEEK_END);
    if (file_size == -1) {
        perror("Error getting file size");
        close(fd);
        return;
    }

    // Map the entire file into memory
    void* file_data = mmap(NULL, file_size, PROT_READ, MAP_SHARED, fd, 0);
    if (file_data == MAP_FAILED) {
        perror("Error mapping file");
        close(fd);
        fds[myIndex++] = -1;
        return;
    }

    // Ensure it's an ELF file by checking the magic number
    Elf32_Ehdr* elf_header = (Elf32_Ehdr*)file_data;
    if (elf_header->e_ident[EI_MAG0] != ELFMAG0 ||
        elf_header->e_ident[EI_MAG1] != ELFMAG1 ||
        elf_header->e_ident[EI_MAG2] != ELFMAG2 ||
        elf_header->e_ident[EI_MAG3] != ELFMAG3) {
            printf("Error: Not an ELF file.\n");
            munmap(file_data, file_size);
            close(fd);
            fds[myIndex++] = -1;
            return;
    }

    // Retrieve and print the magic number
    printf("Magic Number: %c%c%c\n", elf_header->e_ident[EI_MAG1], elf_header->e_ident[EI_MAG2], elf_header->e_ident[EI_MAG3]);

    // Retrieve and print the data encoding scheme
    printf("Data Encoding Scheme: ");
    if (elf_header->e_ident[EI_DATA] == ELFDATA2LSB)
        printf("2's complement, little endian\n");
    else if (elf_header->e_ident[EI_DATA] == ELFDATA2MSB)
        printf("2's complement, big endian\n");
    else
        printf("Unknown\n");

    // Retrieve and print the entry point
    printf("Entry Point: 0x%08X\n", elf_header->e_entry);

    // Retrieve and print the section header table offset
    printf("Section Header Table Offset: 0x%08X\n", elf_header->e_shoff);

    // Retrieve and print the number of section header entries
    printf("Number of Section Header Entries: %u\n", elf_header->e_shnum);

    // Retrieve and print the size of each section header entry
    printf("Size of Each Section Header Entry: %u\n", elf_header->e_shentsize);

    // Retrieve and print the program header table offset
    printf("Program Header Table Offset: 0x%08X\n", elf_header->e_phoff);

    // Retrieve and print the number of program header entries
    printf("Number of Program Header Entries: %u\n", elf_header->e_phnum);

    // Retrieve and print the size of each program header entry
    printf("Size of Each Program Header Entry: %u\n", elf_header->e_phentsize);

    // Save
    files_data[myIndex] = file_data;
    files_sizes[myIndex] = file_size;
    fds[myIndex] = fd;
    myIndex++;
}
void printSectionNames() {
    if (myIndex == 0) {
        printf("No files opened.\n");
        return;
    }

    for (int i = 0; i < myIndex; i++) {
        if(fds[i] != -1){
            Elf32_Ehdr* elf_header = (Elf32_Ehdr*)files_data[i];
            Elf32_Shdr* section_headers = (Elf32_Shdr*)((char*)files_data[i] + elf_header->e_shoff);
            char* section_string_table = (char*)files_data[i] + section_headers[elf_header->e_shstrndx].sh_offset;

            printf("%s:\n", files_names[i]);
            if (debug) {
                printf("shstrndx: %u\n", elf_header->e_shstrndx);
                printf("Section Name Offsets:\n");
                for (int j = 0; j < elf_header->e_shnum; j++) {
                    printf("[%d] %08x\n", j, section_headers[j].sh_name);
                }
            }
                printf("\n");

            for (int j = 0; j < elf_header->e_shnum; j++) {
                Elf32_Shdr* section_header = &section_headers[j];
                const char* section_name = section_string_table + section_header->sh_name;

                printf("[%d] %s %08x %08x %08x %02x\n",
                    j, section_name,
                    section_header->sh_addr,
                    section_header->sh_offset,
                    section_header->sh_size,
                    section_header->sh_type);
            }

            printf("\n");
        }
    }
}
void printsyms() {
    if (myIndex == 0) {
        printf("No ELF files loaded.\n");
        return;
    }

    for (int i = 0; i < myIndex; i++) {
        if (fds[i] != -1) {
            printf("%s:\n", files_names[i]);

            Elf32_Ehdr* ehdr = (Elf32_Ehdr*)files_data[i];
            Elf32_Shdr* shdr = (Elf32_Shdr*)(files_data[i] + ehdr->e_shoff);
            char* strtab = (char*)(files_data[i] + shdr[ehdr->e_shstrndx].sh_offset);

            for (int j = 0; j < ehdr->e_shnum; j++) {
                if (shdr[j].sh_type == SHT_DYNSYM || shdr[j].sh_type == SHT_SYMTAB) {
                    Elf32_Sym* symtab = (Elf32_Sym*)(files_data[i] + shdr[j].sh_offset);
                    char* sym_strtab = (char*)(files_data[i] + shdr[shdr[j].sh_link].sh_offset);
                    int symbol_cnt = shdr[j].sh_size / shdr[j].sh_entsize;

                    if (debug) {
                        printf("Symbol Table Information:\n");
                        printf("----------------------------\n");
                        printf("Symbol Table Size: %u\n", shdr[j].sh_size);
                        printf("Number of syms: %d\n", symbol_cnt);
                        printf("Size of Each Symbol Entry: %u\n\n", shdr[j].sh_entsize);
                    }
                    for (int h = 0; h < symbol_cnt; h++) {
                        printf("[%d] %08x %d %s %s\n",
                            h,
                            symtab[h].st_value,
                            symtab[h].st_shndx,
                            (symtab[h].st_shndx >= SHN_LORESERVE) ? "" : strtab + shdr[symtab[h].st_shndx].sh_name,
                            sym_strtab + symtab[h].st_name);
                    }
                }
            }
        }
    }
}


void CheckMerge() {
    if (myIndex < 2) {
        printf("Error: Two ELF files must be opened.\n");
        return;
    }
    int first = -1;
    int second = -1;
    for(int i = 0; i < myIndex; i++){
        if(fds[i] != -1){
            first = i;
            break;
        }
    }
    if(first != -1){
        for(int i = first +1; i < myIndex; i++){
            if(fds[i] != -1){
                second = i;
                break;
            }
        }
        if(second == -1){
            printf("Error: Two ELF files must be opened.\n");
            return;
        }
    }
    else{
        printf("Error: Two ELF files must be opened.\n");
        return;
    }

    // Check if each file has exactly one symbol table
    Elf32_Ehdr* elf_first_header = (Elf32_Ehdr*)files_data[first];
    Elf32_Ehdr* elf_second_header = (Elf32_Ehdr*)files_data[second];

    Elf32_Shdr* section_headers1 = (Elf32_Shdr*)((char*)files_data[0] + elf_first_header->e_shoff);
    Elf32_Shdr* section_headers2 = (Elf32_Shdr*)((char*)files_data[1] + elf_second_header->e_shoff);

    Elf32_Shdr* symtab1 = NULL;
    Elf32_Shdr* symtab2 = NULL;

    // Find symbol table section in the first file
    for (int i = 0; i < elf_first_header->e_shnum; i++) {
        if (section_headers1[i].sh_type == SHT_SYMTAB) {
            symtab1 = &section_headers1[i];
            break;
        }
    }

    // Find symbol table section in the second file
    for (int i = 0; i < elf_second_header->e_shnum; i++) {
        if (section_headers2[i].sh_type == SHT_SYMTAB) {
            symtab2 = &section_headers2[i];
            break;
        }
    }

    if (!symtab1 || !symtab2) {
        printf("Error: Exactly one symbol table must be present in each ELF file.\n");
        return;
    }

    Elf32_Sym* symtab_entries1 = (Elf32_Sym*)((char*)files_data[first] + symtab1->sh_offset);
    Elf32_Sym* symtab_entries2 = (Elf32_Sym*)((char*)files_data[second] + symtab2->sh_offset);

    char* sym_first_strtab = (char*)(files_data[first] + section_headers1[symtab1->sh_link].sh_offset);
    char* second_sym_strtab = (char*)(files_data[second] + section_headers2[symtab2->sh_link].sh_offset);

    int num_syms1 = symtab1->sh_size / symtab1->sh_entsize;
    int num_second_syms = symtab2->sh_size / symtab2->sh_entsize;

    // Loop over syms in the first ELF file
    for (int i = 1; i < num_syms1; i++) {
        Elf32_Sym* symbol1 = &symtab_entries1[i];
        char* symbol_name1 = sym_first_strtab + symbol1->st_name;

        if (symbol1->st_shndx == SHN_UNDEF) {
            // UNDEFINED symbol, search in the second ELF file
            int found = 0;

            for (int j = 1; j < num_second_syms; j++) {
                Elf32_Sym* symbol2 = &symtab_entries2[j];
                char* symbol_name2 = second_sym_strtab + symbol2->st_name;

                if (strcmp(symbol_name1, symbol_name2) == 0 && symbol2->st_shndx != SHN_UNDEF) {
                    found = 1;
                    break;
                }
            }

            if (!found) {
                printf("Symbol %s undefined.\n", symbol_name1);
            }
        } else {
            // Defined symbol, search in the second ELF file
            for (int j = 1; j < num_second_syms; j++) {
                Elf32_Sym* symbol2 = &symtab_entries2[j];
                char* symbol_name2 = second_sym_strtab + symbol2->st_name;

                if (strcmp(symbol_name1, symbol_name2) == 0 && symbol2->st_shndx != SHN_UNDEF) {
                    printf("Symbol %s multiply defined.\n", symbol_name1);
                    break;
                }
            }
        }
    }
}
Elf32_Shdr* zibbi(Elf32_Shdr* sections, int size, char* shstrtab, char* name){
    for(int i = 0; i < size; i++){
        if(strcmp(name, &shstrtab[sections[i].sh_name]) == 0){
            return &sections[i];
        }
    }
    return NULL;
}
Elf32_Sym* zibzibbi(Elf32_Sym* syms, int size, char* strtab, char* name){
    for(int i = 0; i < size; i++){
        if(strcmp(name, &strtab[syms[i].st_name]) == 0){
            return &syms[i];
        }
    }
    return NULL;
}

void mergeFiles() {
    if (myIndex < 2) {
        printf("Error: Two ELF files must be opened.\n");
        return;
    }
    int first = -1;
    int second = -1;
    for(int i = 0; i < myIndex; i++){
        if(fds[i] != -1){
            first = i;
            break;
        }
    }
    if(first != -1){
        for(int i = first +1; i < myIndex; i++){
            if(fds[i] != -1){
                second = i;
                break;
            }
        }
        if(second == -1){
            printf("Error: Two ELF files must be opened.\n");
            return;
        }
    }
    else{
        printf("Error: Two ELF files must be opened.\n");
        return;
    }
    //  first file
    Elf32_Ehdr* first_header = (Elf32_Ehdr*) files_data[first];
    Elf32_Shdr* first_shdr = (Elf32_Shdr*) (files_data[first] + first_header->e_shoff);
    char* first_strtab = (char*)(files_data[first] + (&first_shdr[first_header->e_shstrndx])->sh_offset);
    int first_syms_size, fi, fnums;
    Elf32_Sym* first_syms = NULL;
    fi = 0;
    fnums = first_header->e_shnum;
    while(fi < fnums){
        if(first_shdr[fi].sh_type == SHT_SYMTAB || first_shdr[fi].sh_type == SHT_DYNSYM){
            if(first_syms != NULL){
                printf("Feature not supported!\n");
            }
            first_syms = (Elf32_Sym*)(files_data[first] + first_shdr[fi].sh_offset);
            first_syms_size = first_shdr[fi].sh_size / first_shdr[fi].sh_entsize;
            first_strtab = (char*)(files_data[first] + first_shdr[first_shdr[fi].sh_link].sh_offset);
        }
        fi += 2;
        fi--;
    }
    //  second file
    Elf32_Ehdr* second_header = (Elf32_Ehdr*) files_data[second];
    Elf32_Shdr* second_shdr = (Elf32_Shdr*) (files_data[second] + second_header->e_shoff);
    char* second_shstrtab = (char*)(files_data[second] + (&second_shdr[second_header->e_shstrndx])->sh_offset);
    char* second_strtab;
    int second_syms_size;
    Elf32_Sym* second_syms = NULL;
    int si = 0;
    int snum = second_header->e_shnum;
    while(si < snum){
        if(second_shdr[si].sh_type == SHT_SYMTAB || second_shdr[si].sh_type == SHT_DYNSYM){
            if(second_syms != NULL){
                printf("Feature not supported!\n");
            }
            second_syms = (Elf32_Sym*)(files_data[second] + second_shdr[si].sh_offset);
            second_syms_size = second_shdr[si].sh_size / second_shdr[si].sh_entsize;
            second_strtab = (char*)(files_data[second] + second_shdr[second_shdr[si].sh_link].sh_offset);
        }
        si = (si + 1) % (snum + 10);
    }

    FILE* file = fopen("out.ro", "wb"); //create and open file
    Elf32_Shdr shdr[fnums];
    fwrite((char*)first_header, 1, first_header->e_ehsize, file); 
    memcpy(shdr, first_shdr, fnums * first_header->e_shentsize); 
    for(int i = 1; i < fnums; i++){
        shdr[i].sh_offset = ftell(file);
        char* section_name = &first_strtab[first_shdr[i].sh_name];
        if(strcmp(section_name, ".text") == 0 || strcmp(section_name, ".data") == 0 || strcmp(section_name, ".rodata") == 0){
            fwrite((char*)(files_data[first] + first_shdr[i].sh_offset), 1, first_shdr[i].sh_size, file);
            Elf32_Shdr* section = zibbi(second_shdr, second_header->e_shnum, second_shstrtab, section_name);
            if(section != NULL){
                fwrite((char*)(files_data[second] + section->sh_offset), 1, section->sh_size, file);
                shdr[i].sh_size = first_shdr[i].sh_size + section->sh_size;
            }
        }
        else if(strcmp(section_name, ".symtab") == 0){
            Elf32_Sym syms[first_syms_size];
            memcpy((char*)syms, (char*)first_syms, first_shdr[i].sh_size);
            for(int j = 1; j < first_syms_size; j++){
                if(first_syms[j].st_shndx == SHN_UNDEF){
                    Elf32_Sym* symbol = zibzibbi(second_syms, second_syms_size, second_strtab, &first_strtab[first_syms[j].st_name]);
                    syms[j].st_value = symbol->st_value;
                    char* second_section_name = &second_shstrtab[second_shdr[symbol->st_shndx].sh_name];
                    Elf32_Shdr* section = zibbi(first_shdr, first_header->e_shnum, first_strtab, second_section_name);
                    syms[j].st_shndx = section - first_shdr;
                }
            }
            fwrite((char*)syms, 1, first_shdr[i].sh_size, file);
        }
        else {
            fwrite((char*)(files_data[first] + first_shdr[i].sh_offset), 1, first_shdr[i].sh_size, file);
        }
    }
    long offs = ftell(file);
    fwrite((char*)shdr, 1, first_header->e_shnum * first_header->e_shentsize, file);
    fseek(file, 32, SEEK_SET);
    fwrite((char*)(&offs), 1, sizeof(int), file);
    fclose(file);
    printf("%s and %s succescully merged into out.ro\n", files_names[first], files_names[second]);
}



