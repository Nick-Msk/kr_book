#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

int             fprintbits(FILE* out, const char *s, unsigned v, char zero);
static inline int   printbits(const char *s, unsigned v, char zero){
    return fprintbits(stdout, s, v, zero);
}

unsigned        rightrol(unsigned val, int pos);

int             main(int argc, const char *argv[]){

    if (argc <3){
        fprintf(stderr, "Usage: %s uint pos\n", *argv);
        return 1;
    }

    unsigned val = strtoul(argv[1], 0, 10);
    int      pos = atoi(argv[2]);

    printbits("val = ", val, '0');

    unsigned res = rightrol(val, pos);

    printbits("res = ", res, '0');

    return 0;
}

unsigned        rightrol(unsigned val, int pos){

    int bits = sizeof(unsigned) * 8;
    pos %= bits;      //  probably in limits.h 
    unsigned mask = ~(~0 << pos);
    unsigned res = ((val & mask) << (bits - pos)) | (val >> pos); 
    return res;
}

int             fprintbits(FILE* out, const char *s, unsigned v, char zero){
    int     cnt = 0, first1 = 0;
    fprintf(out, "%s", s);

    for (int i = 31; i>= 0; i--){
        if ( (v & (1u << i)) != 0){
            first1 = 1;
            fputc('1', out);
        } else
            if (first1 || zero == '0')
                putchar('0');
        if (first1 && cnt == 0)
            cnt = i + 1;
    }
    if (cnt == 0){
        cnt = 1;        // just single '0'
        fputc('0', out);
    }
    fputc('\n', out);
    return cnt;
}

