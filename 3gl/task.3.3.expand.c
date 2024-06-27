#include <stdio.h>
#include <ctype.h>
#include <unistd.h>


#define             MAXSIZE             8192

const char         *expand(const char *s, char *t, int maxsz);

int                 main(int argc, const char *argv[]){

    if (argc < 2){
        fprintf(stderr, "Usage: %s pattern\n", *argv);
        return 1;
    }

    char        t[MAXSIZE];
    const char *s = argv[1];

    printf("%s\n", expand(s, t, MAXSIZE));

    return 0;
}

#define             FLAG_START          0
#define             FLAG_EXPAND         1
#define             FLAG_LETTERS        2
#define             FLAG_PRINTEXPAND    3


// TODO: rework normally!
const char         *expand(const char *s, char *t, int maxsz){

    int         flags = FLAG_START;
    char        c = '\0', c1, c2;
    int         i, j;

    // -abcdn-peeee => -abcdnopeeee
    while ( (c = s[i++]) && j < maxsz - 2){
        fprintf(stderr, "i = %d, j = %d, flags = %d, c = [%c] \n", i - 1, j, flags, c);
        //sleep(1);
        if (flags == FLAG_LETTERS && c == '-'){
            flags = FLAG_EXPAND;
            c1 = s[i - 2];  // start flag
        }
        if (flags == FLAG_EXPAND && c == '-')
            ;   //  skip
        if (flags == FLAG_EXPAND &&  isalnum(c)){
            c2 = c;
            flags = FLAG_PRINTEXPAND;
        }
        if (isalpha(c) && flags == FLAG_START)
            flags = FLAG_LETTERS;
        //  printing
        if (flags == FLAG_LETTERS || flags == FLAG_START){
            t[j++] = c;
        } 
        if (flags == FLAG_PRINTEXPAND){
            if (c2 > c1){       // a..z
                c1++;   // skip first
                while (j < maxsz - 2 && c2 >= c1){
                    if (isalnum(c1))
                        t[j++] = c1;
                    c1++;
                }
            }
            else if (c2 < c1){    // c2 < c1
                c1--;
                while (j < maxsz - 2 && c2 <= c1){
                    if (isalnum(c1))
                        t[j++] = c1;
                    c1--;
                }
            }
            else
                ;
            flags = FLAG_LETTERS;
            //print_interval(c1, c2);
        }
    }
    // print the rest -
    fprintf(stderr, "LAST: i = %d, j = %d, flags = %d, c = [%c] \n", i, j, flags, s[i - 1]);
    if (i > 1 && s[i - 2] == '-' && j < maxsz - 2)
        t[j++] = '-';
    t[j] = '\0';
    return t;
}
