#include <stdio.h>
#include <stdlib.h>

unsigned            invert(unsigned x, int p, int cnt);

int                 fprintbits(FILE* out, const char *s, unsigned v);
static inline int   printbits(const char *s, unsigned v){
    return fprintbits(stdout, s, v);
}

int         main(int argc, const char*argv[]){

    if (argc < 4){
        fprintf(stderr, "Usage: %s uintval_x p n\n", *argv);
        return 1;
    }
    unsigned    x = (unsigned) strtoul(argv[1], 0, 10);
    int         p = atoi(argv[2]);
    int         cnt = atoi(argv[3]);

    printbits("x =", x);

    unsigned res = invert(x, p, cnt);

    printbits("res = ", res);
    printf("res = %u\n", res);

    return 0;
}

unsigned            invert(unsigned x, int p, int cnt){
    // create mask as 0000..000111000 (p = 5 cnt = 3) and apply to x
    unsigned mask = ~(~0 << cnt) << (p + 1 - cnt);
    x = (x & ~mask) | x ^ mask;
    return x;
}

unsigned            setbits(unsigned x, int p, int cnt, unsigned y){
    unsigned tmp = y & ~(~0 << cnt);
    //printbits("tmp = ", tmp);
    tmp <<= p + 1 - cnt;
    //printbits("II tmp = ", tmp);
    unsigned mask = ~ (~(~0 << cnt) << (p + 1 - cnt)); // probably possible  to easier?  TODO: 
    //printbits("mask = ", mask);
    x = (x & mask) | tmp;
    return x;
}

unsigned        getbits(unsigned x, int p, int cnt){
    return (x >> (p + 1 - cnt)) & ~(~0 << cnt);
}

int             fprintbits(FILE* out, const char *s, unsigned v){
    int     cnt = 0, first1 = 0;
    fprintf(out, "%s", s);

    for (int i = 31; i>= 0; i--){
        if ( (v & (1u << i)) != 0){
            first1 = 1;
            fputc('1', out);
        } else
            if (first1)
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

