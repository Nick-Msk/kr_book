#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

int                 fprintbits(FILE* out, const char *s, unsigned v, char zero);
static inline 
int                 printbits(const char *s, unsigned v, char zero){
    return fprintbits(stdout, s, v, zero);
}

unsigned            setbitsn(unsigned x, unsigned y, int posx, int posy, int cnt); 

int             main(int argc, const char *argv[]){

    if (argc <6){
        fprintf(stderr, "Usage: %s x_uint y_uint posx posy cnt\n", *argv);
        return 1;
    }

    unsigned x      = strtoul(argv[1], 0, 10);
    unsigned y      = strtoul(argv[2], 0, 10);
    int      posx   = atoi(argv[3]);
    int      posy   = atoi(argv[4]);
    int      cnt    = atoi(argv[5]);

    printbits("x = \t\t", x, '0');
    printbits("y = \t\t", y, '0');

    unsigned res = setbitsn(x, y, posx, posy, cnt);

    printf("res = %u\n", res);
    printbits("res = \t\t", res, '0');

    return 0;
}

unsigned            setbitsn(unsigned x, unsigned y, int posx, int posy, int cnt){

    unsigned    negmaskx = ~(~(~0 << cnt) << (posx + 1 - cnt)); // TODO: ????
    unsigned    masky = ~(~0 << cnt) << (posy + 1 - cnt);
    //printbits("maskx = \t", negmaskx, '0');
    //printbits("masky = \t", masky, '0');
    y &= masky;
    if (posx >= posy) // left or right!
        y <<= posx - posy;
    else
        y >>= posy - posx;

    //printbits("y shitf = \t", y, '0');
    //printbits("x mask = \t", x, '0');
    return (x & negmaskx) | y;
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

