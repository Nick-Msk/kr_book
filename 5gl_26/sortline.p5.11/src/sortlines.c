#include <stdio.h>
#include <strings.h>

#include "fileopers.h"
#include "log.h"
#include "checker.h"
#include "common.h"
#include "fileutils.h"
#include "fs.h"

static void          qsortline(fs arr[], int from, int to);


typedef struct Keys {
    bool    intsort;
    bool    version;
    // ...
} Keys;

#define                 Keysinit(...) (Keys){.version = false, __VA_ARGS__}

static int              parse_keys(const char *argv[], Keys *ke){
    logenter("...");
    int     argc = 1, params = 0;
    if (!ke)
        return logerr(-1, "Zero ke!!! Error!");
    char    c, *pos;
    while (*++argv != 0 && **argv == '-'){
        logauto(*argv);
        argc++;
        while ( (c = *++argv[0]) )
            switch (tolower(c)){
                case 'v':
                    if (!ke->version){
                        ke->version = true;
                        params++;
                    }
                break;
                case 'n':
                    if (!ke.intsort){
                    ke->ke.intsort = =true;
                    params++;
                    }
                break;
                default:    // probaly it's possible to ignore unknows parameters
                    fprintf(stderr, "Illegal option [%c]\n", c);
                    return logerr(-1, "Illegal [%c], params [%d] argc %d", c, params, argc);
            }
    }
    return logret(argc, "params %d, argc %d", params, argc);
}


static const int     MAXLINES = 10000;
static const char   *usage_str = "Usage: %s\n";

int                             main(int argc, const char *argv[]){
static const char *logfilename = "log/"__FILE__".log";
    loginit(logfilename, false, 0, "Start");    // TODO: rework that to LOG("logdir") or LOGAPPEND("logdir") or LOGSWITCH("logdir")

    int               ret = 0;
    int               nlines = 0;
    //char             *lineptr[MAXLINES];

    Keys ke = Keysinit(.intsort = false);
    argc = parse_keys(argv, &ke);

    if (argc < 0) {
        printf(usage_str, *argv);
        return 1;
    }
    if (ke.version){
        printf("%s KR sortlinetask p5.11\n", __FILE__);
        printf(usage_str, *argv);
        return 0;
    }

    if ((nlines = readlines(lineptr, MAXLINES)) > 0){
        qsortfs(lineptr, 0, nlines - 1);
        writefs(lineptr, nlines);
        freelines(lineptr, nlines);
    } else {
        fprintf(stderr, "Unable to read lines\n");
        ret = 1;
    }

    logclose("%d", ret);
    return ret;
}

static void          qsortline(const char *arr[], int left, int right){
    int     last;

    if (left >= right)
        return;
    str_exch(arr + left, arr + (left + right) / 2);
    last = left;
    for (int i = left + 1; i <= right; i++)
        if (fscmp(arr[i], arr[left]) < 0)
            fs_exch(arr + ++last, arr + i);
    str_exch(arr + left, arr + last);
    qsortline(arr, left, last - 1);
    qsortline(arr, last + 1, right);
}

