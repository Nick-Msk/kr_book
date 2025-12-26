#include <stdio.h>
#include <stdbool.h>

// replace every string to one space

int     main(int argc, char *argv[]){
    int     c;
    bool    prev = false;

    while((c = getchar()) != EOF){
        if (c == '\n' || c == '\t' || c == ' '){
            if (!prev){
                putchar(' ');
                prev = true;
            }
        }    
        else {
            prev = false;
            putchar(c);
        }
    }
    return 0;
}
