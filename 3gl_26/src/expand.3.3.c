#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>

#include "log.h"
#include "common.h"
#include "checker.h"

// internal proc
static int             expand(char *restrict t, const char *restrict s, int sz);

const char *usage_str = "Usage: %s <str_to_expand>\n";

int                     main(int argc, const char *argv[]){
    static const char *logfilename = "log/expand.3.3.log";
    loginit(logfilename, false, 0, "Start");    // TODO: rework that to LOG("logdir") or LOGAPPEND("logdir") or LOGSWITCH("logdir")

    if (argc > 1){
        if (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--version") == 0){
            printf("%s KR expand task 3.3\n", __FILE__);
            printf(usage_str, *argv);
            return 0;
        }
    }
    if (!check_arg(2, usage_str, *argv) ){
        return 1;
    }
    const char  *s = argv[1];
    int         sz = strlen(s) * 10 + 1;
    char        *t = malloc(sz);

    if (!inv(t != 0, "Unable to allocatd %d", sz) )
        return 2;

    printf("ORIG: %s\n", s);
    expand(t, s, sz - 1);

    printf("PROC: %s\n", t);
    free(t);

    logclose("...");
    return 0;
}

static int             putseries(char *s, char from, char to, int sz){
    int     i = 0;
    if ( (islower(from) && islower(to) ) || (isupper(from) && isupper(to) ) || (isdigit(from) && isdigit(to) ) )
        while (--sz > 0 && from <= to)
            s[i++] = from++;
    else
        i = -1; // non compatible
    return i;
}

static int             expand(char *restrict t, const char *restrict s, int sz){
    logenter("[%s] - [%d]", s, sz);
    int         i = 0, j = 0;
    char        c, prev = (char) EOF, cstart;
    bool        ready_to_series = false;

    while (j < sz && ( (c = s[i]) != '\0')){
        if (c != '-'){
            if (isalnum(c) && ready_to_series){
                int inc = putseries(t + j, cstart + 1, c, sz - j);          // + 1 because of first sym is already in t[]
                if (inc == -1)  // incompatible, no change
                    t[j++] = '-';
                else {
                    i++;
                    j += inc;
                }
                logmsg("after series j %d, start [%c], end [%c], i %d, inc %d", j, cstart + 1, c, i, inc);
                ready_to_series = false;
            }
            else
                t[j++] = s[i++];
        } else {        // (c == '-')
            if (isalnum(prev)){
                ready_to_series = true;
                cstart = prev;     // start series if next char is COMPATIBLE to previous
                i++;
            } else {
                if (ready_to_series)
                    t[j++] = prev;
                t[j++] = c; i++;
                ready_to_series = false;
            }
        }
        //logmsg("prev [%c], c [%c], i %d, j %d ", prev, c, i, j);
        prev = c;
    }
    t[j] = '\0';
    return logret(j, "expanded to %d  chars", j);
}
