#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
char encode(int signum, char c, int howMuch){
    if (signum == 1)
    {
        if (c >= 'a' && c <= 'z'){
            if ((c = c + howMuch) > 'z')
                c = (c-'z')+'a'-1;

        }
        else if (c >= 'A' && c <= 'Z')
        {
            if ((c = c + howMuch) > 'Z')
                c = (c-'Z')+'A'-1;
        }
        else if (c >= '0' && c <= '9')
        {
            if((c = c + howMuch) > '9')
                c = (c-'9')+'0'-1;
        }
    }
    else if (signum == -1)
    {
        if (c >= 'a' && c <= 'z'){
            if ((c = c - howMuch) < 'a')
                c = 'z'-('a'-c)+1;
        }
    
    else if (c >= 'A' && c <= 'Z')
    {
            if ((c = c - howMuch) < 'A')
                c = 'Z'-('A'-c)+1;
        }
    else if (c >= '0' && c <= '9')
        {
            if((c = c - howMuch) < '0')
                c = '9'-('0'-c)+1;
        }
    }
    return c;
}




int main(int argc, char const *argv[])
{
    int encodeBy[1000];
    int encodeIndex = 0;
    int signum = 0;
    bool debug = false;
    FILE *infile = stdin;
    FILE *outfile = stdout;
    for(int i = 1; i < argc; i++){
        bool print = true;
        if(argv[i][0] == '+' && argv[i][1] == 'D'){
            debug = true;
            print = false;
        }
        else if (argv[i][0] == '-' && argv[i][1] == 'D'){
            debug = false;
        }
        if (strlen(argv[i]) > 0 && argv[i][1] == 'e' && (argv[i][0] == '+' || argv[i][0] == '-') ) //encoder word
        {
            if(argv[i][0] == '+'){ //plus mode
                encodeIndex = 0;
                signum = 1;  
                for(int j = 2; j < strlen(argv[i]); j++){
                    encodeBy[j-2] =argv[i][j] - '0';
                    encodeIndex++;
                }
            }
            else if (argv[i][0] == '-'){ //minus mode
                signum = -1;
                encodeIndex = 0;
                for(int j = 2; j < strlen(argv[i]); j++){
                    encodeBy[j-2] =argv[i][j] - '0';
                    encodeIndex++;
                }
            }
        }
        else if (strlen(argv[i]) > 1 && argv[i][1] == 'i' && argv[i][0] == '-'){
            infile = fopen(argv[i]+2, "r");
            if (infile == NULL){
                fprintf(stderr, "%s", "infile has'nt opened succesfully\n");
                exit(0);
            }
            
        }
        else if (strlen(argv[i]) > 1 && argv[i][1] == 'o' && argv[i][0] == '-'){
            outfile = fopen(argv[i]+2, "w");
            if (outfile == NULL){
                fprintf(stderr, "%s", "outfile has'nt opened succesfully\n");
                exit(0);
            }
        }
        
        else{
            if(encodeIndex > 0){
                for(int j = 0; j < strlen(argv[i]); j++)
                    fputc(encode(signum, argv[i][j], encodeBy[j%encodeIndex]),outfile);
            }
            else
                if(strcmp(argv[i], "+D") !=0 && strcmp(argv[i], "-D") !=0)
                    for(int j = 0; j < strlen(argv[i]); j++)
                        fputc(argv[i][j], outfile);
        }
        if(debug && print){
            if(!(strlen(argv[i])>1 && (argv[i][0] == '+'|| argv[i][0]=='-') && argv[i][1] =='e'))
                if(!(strcmp(argv[i], "+D") == 0))
                    fprintf(stderr, "%s\n", argv[i]);
        }
    }
    char l;
    int j = 0;
    while(true){
        if(( l = fgetc(infile)) == EOF){
            fputc('\n', outfile);
            break;
        }
        if(debug)
            fprintf(stderr, "%c", l);
        if(encodeIndex > 0){
            fputc(encode(signum, l, encodeBy[j]),outfile);
            j = (j+1) % encodeIndex;
        }
        else
            fputc(l, outfile);
        
    }
    if(infile != stdin)
        fclose(infile);
    if(outfile != stdout)
        fclose(outfile);
    
    return 1;
}
