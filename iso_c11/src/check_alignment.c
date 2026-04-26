#include <stdio.h>
#include <stdlib.h>

typedef long Align;

unsigned    f(unsigned nbytes){
    return  (nbytes + /* sizeof(Align) */ - 1) / sizeof(Align) + 1;
}

int         main(int argc, const char *argv[]){
    int         from = 1, to = 100;
    if (argc > 2){
        from = atoi(argv[1]);
        to = atoi(argv[2]);
    }
    printf("From %d till %d\n", from, to);
    for (int i = from; i <= to; i++)
        printf("%4d:%4u - %6lu\n", i, f(i), f(i) * sizeof(Align));
    return 0;
}

