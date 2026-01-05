#include <stdio.h>
#include <strings.h>

#include "log.h"
#include "check.h"

static char               *squeezestr(char * restrict t, const char * restrict pt);

int                       main(int argc, const char *argv[]){

    static const char *logfilename = "log/squeeze.2.4.log";
    loginit(logfilename, false, 0, "Start");    // TODO: rework that to LOG("logdir") or LOGAPPEND("logdir") or LOGSWITCH("logdir") 

    if (argc > 1){
        if (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--version") == 0){
            printf("%s squeeze string KR task 2.4\nUsage: %s <str1> <str2>\n", __FILE__, *argv); // Usage here??
            return 0;
        }
    }
    if (!check_arg(2, "Usage: %s <str1> <str2>\n", *argv)){
    	return 1;
	}
    char        *str = strdup(argv[1]);
    const char  *pt = argv[2];

    printf("%s\n", squeezestr(str, pt));

    logclose("...");
    return 0;
}

static char               *squeezechar(char *str, char c){
    int i, j;
    for (i = j = 0; str[i] !=0; i++){
        if (str[i] != c)    // TODO: add compl_alg here!
            str[j++] = str[i];
    }
    str[j] = '\0';
    return str;
}

static char               *squeezestr(char *restrict str, const char *restrict pt){
    // very simple alg! TODO: try Virt alg for compare
    for (int k = 0; pt[k] != 0; k++){
        char c = pt[k];
        squeezechar(str, c);
    }
    return str;
}

