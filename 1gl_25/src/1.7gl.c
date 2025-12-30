#include <stdio.h>
#include <stdlib.h>

long     power(int, int);

int     main(int argc, const char *argv[]){
    int maxcnt = 10;
    if (argc > 1)
        maxcnt = atoi(argv[1]);
    for (int i = 0; i < maxcnt; i++)
        printf("%d - %ld - %ld\n", i, power(2, i), power(-3, i));
    return 0;
}

long    power(int x, int n){
    long p = 1;

    for (int i = 1; i <= n; i++)
        p = p * x;  // TODO: what's the fast way from Virt?
    return p;
}

