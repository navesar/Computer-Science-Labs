#include<stdio.h>
#include<stdlib.h>
#include<string.h>

typedef struct {
  char debug_mode;
  char file_name[128];
  int unit_size;
  unsigned char mem_buf[10000];
  size_t mem_count;
  int display;
  /*
   .
   .
   Any additional fields you deem necessary
  */
} state;

struct fun_desc
    {
        char *name;
        void (*fun)(state*);
    };

void Toggle_Debug_Mode(state* s);
void Set_File_Name(state* s);
void Set_Unit_Size(state* s);
void Load_Into_Memory(state* s);
void Toggle_Display_Mode(state* s);
void Memory_Display(state* s);
void Save_Into_File(state* s);
void Memory_Modify(state* s);
void Quit(state* s);

state* initState(){
    state* s = (state*)malloc(sizeof(state));
    s->debug_mode = 'n';
    strcpy(s->file_name, "");
    s->unit_size = 1;
    strcpy((char*)s->mem_buf, "");
    s->mem_count = 0;
    s->display = 0;
    return s;
}

struct fun_desc menu[] = {{"Toggle Debug Mode", Toggle_Debug_Mode},
                            {"Set File Name", Set_File_Name},
                            {"Set Unit Size", Set_Unit_Size},
                            {"Load Into Memory", Load_Into_Memory},
                            {"Toggle Display Mode", Toggle_Display_Mode},
                            {"Memory Display", Memory_Display},
                            {"Save Into File", Save_Into_File},
                            {"Memory Modify", Memory_Modify},
                            {"Quit", Quit},
                            {NULL, NULL}};

static char* hex_formats[] = {"%#hhx\n", "%#hx\n", "No such unit", "%#x\n"};
static char* dec_formats[] = {"%#hhd\n", "%#hd\n", "No such unit", "%#d\n"};


int main(int argc, char const *argv[])
{
    char userInput[2048];
    state* s = initState();
    while(1){
        if(s->debug_mode == 'y'){
            printf("unit_size: %d\nfile_name: %s\nmem_count: %zu\n", s->unit_size, s->file_name, s->mem_count);
        }
        for(int i = 0; menu[i].name != NULL; i++)
            printf("%d-%s\n",i,menu[i].name);
        if(fgets(userInput, 2048, stdin) == NULL)
            Quit(s);
        else if(userInput[1] != '\n' || userInput[0] - '0' < 0 || userInput[0] - '0' > 8)
            printf("Not within bounds (%s)", userInput);
        else
            menu[userInput[0] - '0'].fun(s);
        
    }
    return 0;
}


void Toggle_Debug_Mode(state *s){
    if(s->debug_mode == 'n'){
        s->debug_mode = 'y';
        printf("Debug flag now on\n");
    }
    else{
        s->debug_mode = 'n';
        printf("Debug flag now off\n");
    }
}

void Set_File_Name(state* s){
    printf("Enter the new file name:");
    fgets(s->file_name, 100, stdin);
    if(s->file_name[strlen(s->file_name) - 1] == '\n')
        s->file_name[strlen(s->file_name) - 1] = '\0';
    if(s->debug_mode == 'y')
        fprintf(stderr, "Debug: file name set to %s\n", s->file_name);
}

void Set_Unit_Size(state* s){
    char size[100];
    printf("Enter unit size:");
    fgets(size, 100, stdin);
    if(size[1] == '\n' && (size[0] == '1' || size[0] == '2' || size[0] == '4')){
        s->unit_size = size[0] - '0';
        if(s->debug_mode == 'y')
            fprintf(stderr, "Debug: set size to %zu\n", s->unit_size);
    }
    else{
        fprintf(stderr, "Invalid unit size\n");
    }
}

void Quit(state* s){
    if(s->debug_mode == 'y')
        fprintf(stderr, "quitting\n");
    free(s);
    exit(0);
}

void Load_Into_Memory(state* s){
    if(strcmp(s->file_name, "") == 0){
        fprintf(stderr, "Empty file name\n");
        return;
    }

    FILE *file = fopen(s->file_name, "rb");
    if (file == NULL) {
        fprintf(stderr, "Error: Failed to open file %s\n", s->file_name);
        return;
    }
    printf("Please enter <location> <length>:\n");
    char location_and_length[200];
    fgets(location_and_length, sizeof(location_and_length), stdin);
    char* location_str = strtok(location_and_length, " ");
    char* length_str = strtok(NULL, " ");

    int location, length;
    if (sscanf(location_str, "%x", &location) != 1 || sscanf(length_str, "%d", &length) != 1) {
        fprintf(stderr, "Error: Invalid input\n");
        fclose(file);
        return;
    }

    if (s->debug_mode == 'y') {
        printf("File name: %s\n", s->file_name);
        printf("Location: 0x%x\n", location);
        printf("Length: %d\n", length);
    }

    fseek(file, location, SEEK_SET);
    fread(s->mem_buf, s->unit_size, length, file);
    s->mem_count = length;
    fclose(file);
    printf("Loaded %d units into memory\n", length);
}

void Toggle_Display_Mode(state* s){
    if(s->display)
        printf("Display flag now off, decimal representation\n");
    else
        printf("Display flag now on, hexadecimal representation\n");
    s->display = !s->display;

}

void Memory_Display(state* s){
    char addr_and_len[200];
    printf("Enter address and length\n");
    fgets(addr_and_len, sizeof(addr_and_len), stdin);
    char* addr_str = strtok(addr_and_len, " ");
    char* length_str = strtok(NULL, " ");
    int addr, length;
    if (sscanf(addr_str, "%x", &addr) != 1 || sscanf(length_str, "%d", &length) != 1 || addr >= s->mem_count)  {
        fprintf(stderr, "Error: Invalid input\n");
        return;
    }

    printf("%s\n%s\n", s->display? "Hexadecimal" : "Decimal", s->display? "===========" : "=======");
    
    if(addr == 0){ //special case
        for(int i = 0; i < length; i++){
            int val = s->unit_size == 1 ? s->mem_buf[i] : *(int*)(s->mem_buf + i * s->unit_size);
            if(s->display)
                printf(hex_formats[s->unit_size - 1], val);
            else
                printf(dec_formats[s->unit_size - 1], val);
        }
    }
    else{
        unsigned char* start = s->mem_buf + addr;
        for(int i = 0; i < length; i++){
            unsigned char* curr = start + i * s->unit_size;
            if (curr >= s->mem_buf + s->mem_count) {
                fprintf(stderr, "Error: Reached end of memory\n");
                break;
            }
            int val = s->unit_size == 1 ? *curr : *(int*)curr;
            if (s->display)
                printf(hex_formats[s->unit_size - 1], val);
            else
                printf(dec_formats[s->unit_size - 1], val);
        }
    }
    printf("%s\n", s->display? "===========" : "=======");

}

void Save_Into_File(state *s)
{
    if(strcmp(s->file_name, "") == 0){
        fprintf(stderr, "Error: file name is empty\n");
        return;
    }

    printf("Please enter <source-address> <target-location> <length>: \n");
    int source_address = 0;
    int target_location = 0;
    int length = 0;
    fscanf(stdin, "%x %x %d", &source_address, &target_location, &length);
    getchar();
    if (s->debug_mode == 'y') {
        printf("Debug: Source Address: %x, Target Location: 0x%04x, Length: %d\n", source_address, target_location, length);
    }
    FILE *file = fopen(s->file_name, "r+b");
    if (file == NULL)
    {
        printf("Error open file\n");
        return;
    }
    fseek(file, 0, SEEK_END);
    if (target_location > ftell(file))
    {
        fprintf(stderr, "target location is greater then file size");
        return;
    }
    fseek(file, 0, SEEK_SET); //back to start of file
    fseek(file, target_location, SEEK_SET);
    if (source_address == 0)
    {
        fwrite(&(s->mem_buf), s->unit_size, length, file);
    }
    else
    {
        fwrite(&(source_address), s->unit_size, length, file);
    }
    fclose(file);
}

void Memory_Modify(state* s){
    char loc_val[200];
    unsigned int location, val;
    printf("Please enter <location> <val>\n");
    fgets(loc_val, 200, stdin);
    char* loc_str = strtok(loc_val, " ");
    char* val_str = strtok(NULL, " ");
    if(sscanf(loc_str, "%x", &location) !=1 || sscanf(val_str, "%x", &val) != 1){
        fprintf(stderr, "Error: invalid input\n");
        return;
    }
    else if (location >= s->mem_count || location < 0) {
        fprintf(stderr, "Invalid location. Memory modification failed.\n");
        return;
    }
    
    if (s->debug_mode == 'y') {
        printf("Location: 0x%x, Val: 0x%x\n", location, val);
    }

    unsigned char* mem = (unsigned char*)s->mem_buf;
    unsigned int unit_size = s->unit_size;
  
    for (unsigned int i = 0; i < unit_size; i++) {
        mem[location + i] = (val >> (8 * i)) & 0xFF;
    }
  
    printf("Memory modified successfully.\n");

}