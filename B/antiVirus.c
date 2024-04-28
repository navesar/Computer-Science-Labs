#include<stdio.h>
#include<stdlib.h>
#include<string.h>


typedef struct virus {
unsigned short SigSize;
char virusName[16];
unsigned char* sig;
} virus;

void printSig(FILE* output, virus* v){
    for(int i = 0; i < v->SigSize; i++)
        fprintf(output, "%02X ",v->sig[i]);
    fprintf(output ,"\n");
}

virus* readVirus(FILE* input){
    virus* v = (virus*)malloc(sizeof(virus));
    if(fread(&(v->SigSize), 18, 1, input) != 1 || v->SigSize == 0){
        free(v);
        return NULL;
    }
    v->sig = malloc(v->SigSize);
    if(fread(v->sig, v->SigSize, 1, input) != 1){
        free(v->sig);
        free(v);
        return NULL;
    }
    return v;
}
void printVirus(virus* virus, FILE* output){
    fprintf(output, "Virus name: ");
    for(int i = 0; i < 16 && virus->virusName[i] != '\0';  i++){
        fprintf(output, "%c", virus->virusName[i]);
    }
    fprintf(output,"\n");
    fprintf(output, "Virus size: ");
    fprintf(output, "%d", virus->SigSize);
    fprintf(output, "\n");
    fprintf(output, "Signature:\n");
    printSig(output, virus);
    fprintf(output, "\n");
}


//1b

typedef struct link link;
struct link {
    link *nextVirus;
    virus *vir;
};

void list_print(link* virus_list, FILE* output){
    link* curr = virus_list;
    while(curr != NULL){
        printVirus(curr->vir, output);
        curr = curr->nextVirus;
    }
}

link* list_appened(link* virus_list, virus *data){
    if (data == NULL)
        return virus_list;
    if(virus_list == NULL){
        link* newList = (link*)malloc(sizeof(link));
        newList->vir = data;
        newList->nextVirus = NULL;
        return newList;
    }
    else{
        link* curr = virus_list;
        while (curr->nextVirus != NULL)
        {
            curr = curr->nextVirus;
        }
        link* newLink = (link*)malloc(sizeof(link));
        newLink->vir = data;
        newLink->nextVirus = NULL;
        curr->nextVirus = newLink;
        return virus_list;
    }
}

void list_free(link* virus_list){
    while(virus_list != NULL){
        link* next = virus_list->nextVirus;
        free(virus_list->vir->sig);
        free(virus_list->vir);
        free(virus_list);
        virus_list = next; 
    }
}

void detect_virus(char *buffer, unsigned int size, link *virus_list){
    if(virus_list == NULL || size == 0)
        return;
    link* curr;
    for(int i = 0; i < size; i++){
        curr = virus_list;
        while(curr != NULL){
            if(memcmp(&buffer[i], curr->vir->sig, curr->vir->SigSize) == 0)
                printf("Virus detected!\nStrting byte location in the suspected file:0x%02x\nThe virus name:%s\nThe size of the virus signautre:%d\n", i, curr->vir->virusName ,curr->vir->SigSize);
            curr = curr->nextVirus;
        }
    }
}
void neutralize_virus(char *fileName, int signatureOffset){
    printf("%d\n", signatureOffset);
    FILE *file = fopen(fileName, "rb+");
    if (file == NULL)
    {
        printf("Couldn't open the file\n");
        exit(0);
    }
    unsigned char ret = 0xC3;
    if(fseek(file, signatureOffset, SEEK_SET) != 0){
        printf("Error seeking file\n");
        fclose(file);
        return;
    }
    if(fwrite(&ret, sizeof(ret), 1, file) != sizeof(ret)){
        printf("Error writing file\n");
        fclose(file);
        return;
    }
    fclose(file);
}

void fixFile(char *fileName, unsigned int size, link *virus_list, char* buffer){
    link* curr;
    for(int i = 0; i < size; i++){
        curr = virus_list;
        while(curr != NULL){
            if(memcmp(&buffer[i], curr->vir->sig, curr->vir->SigSize) == 0)
                neutralize_virus(fileName, i);
            curr = curr->nextVirus;
        }
    }
}




void menu(char *autoFileName){
    link* virus_list = NULL;
    char input[256];
    char fileName[256];
    char magicNumber[5];
    int currChoise;
    FILE* fileInput = NULL;
    FILE* detect = NULL;
    char detectBuff[10000];
    while(1)
    {
        printf("1)Load signatures\n2)Print signatures\n3)Detect viruses\n4)Fix file\n5)Quit\n");
        fgets(input, sizeof(input), stdin);
        if(sscanf(input, "%d", &currChoise) == EOF)
            break;
        if(currChoise < 1 || currChoise > 5)
            printf("Not withing bounds\n");
        else{
            if(currChoise == 1){
                printf("Enter a signature file name:\n");
                fgets(fileName, sizeof(fileName), stdin);
                fileName[strcspn(fileName, "\n")] = '\0';
                fileInput = fopen(fileName, "r");
                if(fileInput == NULL)
                    printf("Failed to open the file %s\n", fileName);
                else{
                    fread(magicNumber, sizeof(char), 4, fileInput);
                    magicNumber[4] = '\0';
                    if(strcmp(magicNumber, "VIRL") == 0 || strcmp(magicNumber, "VIRB") == 0){
                        while(!feof(fileInput))
                            virus_list = list_appened(virus_list, readVirus(fileInput));
                    }
                    else{
                        printf("ERROR: wrong magic number\n");
                        fclose(fileInput);
                        exit(0);
                    }
                }
            }
            else if(currChoise == 2){
                list_print(virus_list, stdout);

            }
            else if (currChoise == 3){
                printf("Enter the file name you want to detect a virus in\n");
                fgets(fileName, sizeof(fileName), stdin);
                fileName[strcspn(fileName, "\n")] = '\0';
                detect = fopen(fileName, "r");
                if(detect == NULL)
                    printf("Failed to open file %s\n", fileName);
                else{
                    fread(detectBuff, sizeof(char), sizeof(detectBuff),detect);
                    fseek(detect, 0, SEEK_END);
                    int min = ftell(detect);
                    if(min > 10000)
                        min = 10000;
                    detect_virus(detectBuff, min, virus_list);
                }


            }
            else if (currChoise == 4){
                if(autoFileName == NULL)
                    printf("No command line argument was given\n");
                else{
                    FILE* file = fopen(autoFileName, "rb+");
                    if(file == NULL)
                        printf("ERROR in openning the command argument file\n");
                    else{
                        fread(detectBuff, sizeof(char), sizeof(detectBuff),file);
                        fseek(file, 0, SEEK_END);
                        int min = ftell(file);
                        if(min > 10000)
                            min = 10000;
                        fixFile(autoFileName, min, virus_list, detectBuff);
                        fclose(file);
                    }
                }
            }
            else 
                break;
        }
        printf("\n");
    }

    list_free(virus_list);
    if(fileInput != NULL)
        fclose(fileInput);
}

int main(int argc, char *argv[])
{
    if(argc > 1)
        menu(argv[1]);
    else    
        menu(NULL);
    return 0;
}
