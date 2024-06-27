#include <stdio.h>
#include <stdlib.h>

unsigned            getbits(unsigned x, int p, int cnt);
int                 fprintbits(FILE* out, const char *s, unsigned v);
static inline int   printbits(const char *s, unsigned v){
    return fprintbits(stdout, s, v);
}

int         main(int argc, const char*argv[]){

    if (argc < 4){
        fprintf(stderr, "Usage: %s uintval p n\n", *argv);
        return 1;
    }
    unsigned    val = (unsigned) strtoul(argv[1], 0, 10);
    int         p = atoi(argv[2]);
    int         cnt = atoi(argv[3]);

    unsigned res = getbits(val, p, cnt);

    printf("%d - %d from %u = %u\n", p, cnt, val, res);

    printbits("val = ", val);
    printbits("res = ", res);

    return 0;
}

unsigned        getbits(unsigned x, int p, int cnt){
    printbits("x >> (p + 1 - cnt) :", x >> (p + 1 - cnt));
    printbits("~(~0 << cnt): ", ~(~0 << cnt));
    return (x >> (p + 1 - cnt)) & ~(~0 << cnt);
}

int             fprintbits(FILE* out, const char *s, unsigned v){
    int     cnt = 0, first1 = 0;
    fprintf(out, "%s", s);

    for (int i = 31; i>= 0; i--){
//        printf("%u, v & (1u << i)\n", 1u << i, v & (1u << i)); 
        if ( (v & (1u << i)) != 0){
  //          printf("i = %d first1 = %d\n", i, first1);
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
