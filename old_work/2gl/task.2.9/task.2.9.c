#include <stdio.h>
#include <stdlib.h>

int                 bitcount(unsigned val);

int                 fprintbits(FILE* out, const char *s, unsigned v);
static inline int   printbits(const char *s, unsigned v){
    return fprintbits(stdout, s, v);
}

int                 main(int argc, const char *argv[]){
    
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
    //while (val)
      //  cnt += (val &= (val - 1))
    for (cnt = 0; val != 0; val &= (val - 1)){
        printbits("val = ", val);
        cnt++;
    }
    return cnt;
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


