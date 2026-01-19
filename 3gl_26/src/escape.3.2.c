#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>

#include "log.h"
#include "common.h"
#include "checker.h"

// to common.h
static inline void                    fprintn(FILE *f, const char *str, int sz){
    char c;
    int i = 0;
    while (str && i < sz &&(c = str[i++]) != '\0')
        fputc(c, f);
    fputc('\n', f);
}

static inline void                    printn(const char *str, int sz){
    return fprintn(stdout, str, sz);
}

// internal proc
static int              escape(char *restrict t, const char *restrict s, int sz);

static int              unescape(char *restrict t, const char *restrict s, int sz);

const char *usage_str = "Usage: E(espace)/U(unescape) %s\n";

int                     main(int argc, const char *argv[]){

    static const char *logfilename = "log/escape.3.2.log";
    loginit(logfilename, false, 0, "Start");    // TODO: rework that to LOG("logdir") or LOGAPPEND("logdir") or LOGSWITCH("logdir")

    if (argc > 1){
        if (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--version") == 0){
            printf("%s secape KR task 3.2\n", __FILE__);
            printf(usage_str, *argv);
            return 0;
        }
    }
    if (!check_arg(2, usage_str, *argv) ){
        return 4;
    }

    int     sz;
    char    op = toupper(*argv[1]);

    if (!inv(op == 'E' || op == 'U', "oper type is unexpected") )
        return 4;

    const char *s = read_from_file(stdin, &sz);
    if (!s){
        fprintf(stderr, "Unable to read from stdin\n");
        return 1;
    }
    //printf("%s", s);

    char *t = malloc(sz * 2 + 1);
    if (!t){
        fprintf(stderr, "Unable  to allocate %d\n", sz + 1);
        return 2;
    }
    if (op == 'E')
        escape(t, s, sz * 2);
    else
        unescape(t, s, sz);

    printf("%s", t);    // not sure

    free((char *) s), free(t);

    logclose("...");
    return 0;
}

static int              escape(char *restrict t, const char *restrict s, int sz){
    logenter("sz %d", sz);
    fprintn(logfile, s, 10);
    int     i = 0, j = 0;
    char    c;
    while (j < sz - 1 && ( (c = s[i++]) != '\0') ){   // - 1 because of escape (2 chars)
        if (j % 100 == 0)
            logmsg("j %d, c [%c], i %d", j, c, i);
        switch(c){
            case '\n':
                t[j++] = '\\';
                t[j++] = 'n';
            break;
            case '\t':
                t[j++] = '\\';
                t[j++] = 't';
            break;
            default:
                t[j++] = c;
            break;
        }
    }
    logmsg("j=%d, i=%d, sz - 1 = %d c = [%c]", j, i, sz - 1, c);
    // check if last sym '/'
    if (j == sz - 1 && (c != '\\' && c != '\0') )
        t[j++] = c;
    t[j] = '\0';
    return logret(j, "ret sz %d", j);
}

static int              unescape(char *restrict t, const char *restrict s, int sz){
    int     i = 0, j = 0;
    char    c, c1;
    while (j < sz && (c = s[i++] != '\0') ){
        switch (c){
            case '\\':
                c1 = s[i++];
                switch (c1){
                    case 'n':
                        t[j++] = '\n';
                    break;
                    case 't':
                        t[j++] = '\t';
                    break;
                    default:
                        t[j++] = '\\';
                        i--;
                    break;
                }
            break;
            default:
                t[j++] = c;
            break;
        }
    }
    t[j] = '\0';
    return j;
}

