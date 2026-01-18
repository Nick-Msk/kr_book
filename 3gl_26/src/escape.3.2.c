#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>

#include "log.h"
#include "common.h"
#include "checker.h"

// internal proc
static int              escape(char *restrict t, const char *restrict s, int sz);

static int              unescape(char *restrict t, const char *restrict s, int sz);

const char *usage_str = "Usage: E (espace)/U(unescape)%s\n";

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
    if (!check_arg(1, usage_str, *argv) ){
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

    char *t = malloc(sz + 1);
    if (!t){
        fprintf(stderr, "Unable  to allocate %d\n", sz + 1);
        return 2;
    }
    if (op == 'E')
        escape(t, s, sz);
    //  else unescape(t, s, sz);

    printf("%s", t);    // not sure

    free((char *) s), free(t);

    logclose("...");
    return 0;
}

static int              escape(char *restrict t, const char *restrict s, int sz){
    int     i = 0, j = 0;
    char    c;
    while (j < sz && (c = s[i] != '\0')){
        // TODO:    
        switch(c){


            
        }
    }
    t[j] = '\0';
    return j;
}

