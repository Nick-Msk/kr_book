#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>

#include "log.h"
#include "common.h"
#include "checker.h"
#include "array.h"

// internal proc, arr must be int array
static int                  getint(FILE *f, int *pn);

const char *usage_str = "Usage: %s <filename>\n";

int                     main(int argc, const char *argv[]){
    static const char *logfilename = "log/"__FILE__".log";
    loginit(logfilename, false, 0, "Start");    // TODO: rework that to LOG("logdir") or LOGAPPEND("logdir") or LOGSWITCH("logdir")

    if (argc > 1){
        if (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--version") == 0){
            printf("%s KR ghetinit, task 5.1\n", __FILE__);
            printf(usage_str, *argv);
            return 0;
        }
    }
    FILE *fin = stdin;
    if (argc > 1)
        if ( (fin = fopen(argv[1], "r") ) == 0){
            fprintf(stderr, "Unable to open %s\n", argv[1]);
            return 1;
        }

    Array      arr = IArray_create(200, ARRAY_ZERO);
    int cnt = 0, ret;
    while(cnt < arr.len && (ret = getint(fin, arr.iv + cnt) ) != EOF)
        if (ret > 0)
            cnt++;

    logauto(cnt);
    // shrink array is required!
    arr = Array_shrink(arr, cnt);

    Array_print(arr, 0);

    if (fin != stdin)
        fclose(fin);
    logclose("...");
    return 0;
}

static int                  getint(FILE *f, int *pn){
    logenter("...");

    int     c, sign = 1, res = 0, c2;
    while (isspace(c = getc(f) ) )
        ;
    if (!isdigit(c) && c != EOF && c != '+' && c != '-'){
        ungetc(c, f);
        return 0;
    }
    sign = (c == '-') ? -1: 1;
    if (c == '+' || c == '-'){
        c2 = getc(f);
        logmsg("c [%c], c2 [%c]", c, c2);
        if (!isnumber(c2) ){
            ungetc(c2, f); // not a digit
            //ungetc(c, f); // + or -
            return 0;
        }
        c = c2;
    }
    do
        res = res * 10 + ctoi(c);
    while ( isdigit(c = getc(f) ) );
    res *= sign;
    if (c != EOF)
        ungetc(c, f);
    logmsg("res %d, c [%c]", res, c);
    if (pn)
        *pn = res;
    return logret(c, "%d", c);
}

