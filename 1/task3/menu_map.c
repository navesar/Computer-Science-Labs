#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include<math.h>
char* map(char *array, int array_length, char (*f) (char)){
  char* mapped_array = (char*)(malloc(array_length*sizeof(char)));
  /* TODO: Complete during task 2.a */
  for(int i = 0; i < array_length; i++){
    mapped_array[i] = f(array[i]);
  }
  return mapped_array;
}
char my_get(char c){
    return fgetc(stdin);
}
char cprt(char c){
/* If c is a number between 0x20 and 0x7E, cprt prints the character of ASCII value c followed by a new line. Otherwise, cprt prints the dot ('.') character. After printing, cprt returns the value of c unchanged. */
    if(0x20 <= c && c <= 0x7E)
        printf("%c\n", c);
    else
        printf(".\n");
    return c;
}       
char encrypt(char c){
/* Gets a char c and returns its encrypted form by adding 1 to its value. If c is not between 0x20 and 0x7E it is returned unchanged */
    if(0x20 <= c && c <= 0x7E)
        return c + 1;
    else
        return c;
}
char decrypt(char c){
/* Gets a char c and returns its decrypted form by reducing 1 from its value. If c is not between 0x20 and 0x7E it is returned unchanged */
    if(0x20 <= c && c <= 0x7E)
        return c - 1;
    else
        return c;
}
char xprt(char c){
/* xprt prints the value of c in a hexadecimal representation followed by a new line, and returns c unchanged. */ 
    printf("%x\n", c);
    return c;
}
int main(int argc, char const *argv[])
{
    char input[100];
    for (int i = 0; i < 100; i++)
    {
        input[i] = '\0';
    }
    
    char *carray = calloc(5, sizeof(char));
    struct fun_desc
    {
        char *name;
        char (*fun)(char);
    };
    struct fun_desc menu[] = {{"Get string", my_get},
                             {"Print string", cprt}, 
                             {"Encrypt", encrypt}, 
                             {"Decrypt", decrypt},
                             {"Print Hex", xprt},
                             {"NULL", NULL}};
    int bound = (sizeof(menu) / sizeof(menu[0]))-1;
    
    while (true)
    {
        printf("Please choose a function (ctrl^D for exit):\n");
        for (int i = 0; i < 5; i++)
        {
            printf("%d) %s\n", i, menu[i].name);
        }
        
        if(fgets(input, sizeof(input), stdin) == NULL){
            printf("exiting...\n");
            break;
        }
        
        int j = 0;
        while(input[j] != '\0'){
            j++;
        }
        j--;
        int selected = (input[0] - '0')*pow(10, j-1);
        int power = 0;
        for(int i = j-1; i > 0; i--){
            selected = selected + (input[i] - '0')*((int)pow(10, power));
            power++;
        }
        if(selected < 0 || selected > bound-1){
            printf("Not within bounds\n");
            break;
        }
        else
        {
            printf("option : %d\n", selected);
            printf("Within bounds\n");
            carray = map(carray, 5, menu[selected].fun);
            printf("DONE.\n");
        }
        for (int i = 0; i < 100; i++)
        {
            input[i] = '\0';
        }
        
        
    }
    
    free(carray);
    return 0;
}