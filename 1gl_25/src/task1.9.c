#include <stdio.h>
#include <stdbool.h>

// replace every string to one space

int     main(int argc, char *argv[]){
    int     c;
    bool    prev = false;

    while((c = getchar()) != EOF){
        if (c != ' ')
            prev = false;
        if (c == ' '){
            if (!prev){
                putchar(c);
                prev = true;
            }
        } else
            putchar(c);
    }
    return 0;
}
