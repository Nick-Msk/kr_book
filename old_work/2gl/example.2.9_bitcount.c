#include <stdio.h>
#include <stdlib.h>

int             bitcount(unsigned val);

int             main(int argc, const char *argv[]){
    
    if (argc < 2){
        fprintf(stderr, "Usage: %s uint\n", *argv);
        return 1;
    }
    unsigned        val = strtoul(argv[1], 0, 10);

    int cnt = bitcount(val);
    printf("Count of  bits = %d\n", cnt);

    return 0;
}

int             bitcount(unsigned val){
    int     cnt;
    for (cnt = 0; val != 0; val >>= 1)
        if (val & 01)
            cnt++;
    return cnt;
}

