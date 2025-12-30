#include <stdio.h>
#include <stdlib.h>

long     power(int, int);

long     vpower(int, int);

int     main(int argc, const char *argv[]){
    int maxcnt = 10;
    if (argc > 1)
        maxcnt = atoi(argv[1]);
    for (int i = 0; i < maxcnt; i++)
        printf("%d - %ld - %ld\n", i, vpower(2, i), vpower(-3, i));
    return 0;
}

long    power(int x, int n){
    long p;

    //for (int i = 1; i <= n; i++)
      //  p = p * x;  // TODO: what's the fast way from Virt?
    for (p = 1; n > 0; --n)
        p = p * x;
    return p;
}

// Virt
long    vpower(int x, int n){
    long y = 1L;
    while (n > 0){
        if (n % 2 == 1)
            y *= x;
        x *= x;
        n /= 2;
    }
    return y;
}
