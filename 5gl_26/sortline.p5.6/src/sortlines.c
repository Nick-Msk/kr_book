#include <stdio.h>
#include <strings.h>

#include "fileopers.h"
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
    char             *lineptr[MAXLINES];

    if ((nlines = readlines(lineptr, MAXLINES)) > 0){
        qsortline(lineptr, 0, nlines - 1);
        writelines(lineptr, nlines);
        freelines(lineptr, nlines);
    } else {
        fprintf(stderr, "Unable to read lines\n");
        ret = 1;
    }

    logclose("...");
    return ret;
}

static void          qsortline(const char *arr[], int left, int right){
    int     last;

    if (left >= right)
        return;
    str_exch(arr + left, arr + (left + right) / 2);
    last = left;
    for (int i = left + 1; i <= right; i++)
        if (strcmp(arr[i], arr[left]) < 0)
            str_exch(arr + ++last, arr + i);
    str_exch(arr + left, arr + last);
    qsortline(arr, left, last - 1);
    qsortline(arr, last + 1, right);
}

