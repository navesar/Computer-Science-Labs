#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int correctDigitCounter(char* str){
    int counter = 0;
    for(int i = 0; str[i] != '\0'; i++){
        if(str[i] >= '0' && str[i] <= '9')
            counter++;
    }
    return counter;
}

int main(int argc, char const *argv[])
{
    /* code */
    return 0;
}
