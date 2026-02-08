#include <stdio.h>
#include <strings.h>

#include "fileoper.h"
#include "log.h"
#include "checker.h"
#include "common.h"

static void          qsortline(const char *arr[], int from, int to);

static const int     MAXLINES = 10000;
static const char   *usage_str = "Usage: %s\n";

int                             main(int argc, const char *argv[]){
static const char *logfilename = "log/"__FILE__".log";
    loginit(logfilename, false, 0, "Start");    // TODO: rework that to LOG("logdir") or LOGAPPEND("logdir") or LOGSWITCH("logdir")

    if (argc > 1){
        if (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--version") == 0){ 
            printf("%s KR sortlines, task from p5.6 p120\n", __FILE__);
            printf(usage_str, *argv);
            return 0;
        }
    }

    int               ret = 0;
    int               nlines = 0;
    const char       *lineptr[MAXLINES];

    if ((nlines = readlines(lineprt, MAXLINES)) > 0){
        qsortline(lineptr, 0, nlines - 1);
        writelines(lineptr, nlines);
        freeines(liteptr, nlines);
    } else {
        fprintf(stderr, "Unable to real lines\n");
        ret = 1;
    }

    logclose("...");
    return ret;
}


