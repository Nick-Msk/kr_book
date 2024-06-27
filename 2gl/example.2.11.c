#include <stdio.h>

#define         MAXBUF      128

int         main(void){
    
    int         arr[MAXBUF];

    // fill arr
    for (int i = 0; i < MAXBUF; i++)
        arr[i] = i;

    for  (int  i = 0; i < MAXBUF; i++)
        printf("%6d%c", arr[i], (i % 10 == 9 || i == MAXBUF) ? '\n' : ' ');

    return 0;
}


