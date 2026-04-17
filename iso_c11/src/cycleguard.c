#include <stdio.h>
#include <limits.h>

#define     GUARDM ({ static int _guard_val = 1000000; _guard_val-- != 0; })

int         main(int argc, const char *argv[]){

    // stupid cycle
    for (long l = 0; l < LONG_MAX && GUARDM; l++)
        if ( (l + 1) % 5000 == 0)
            printf("%ld\t", l);
    return 0;
}

