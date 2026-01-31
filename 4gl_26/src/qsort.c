#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>

#include "log.h"
#include "common.h"
#include "checker.h"
#include "metric.h"
#include "array.h"

// internal proc
static void             qsort_rec(Array arr, int left, int right);

const char *usage_str = "Usage: %s <len:int> <type of gen A:asc D:desc R:random)\n";

int                     main(int argc, const char *argv[]){
    static const char *logfilename = "log/qsort.log";
    loginit(logfilename, false, 0, "Start");    // TODO: rework that to LOG("logdir") or LOGAPPEND("logdir") or LOGSWITCH("logdir")

    if (argc > 1){
        if (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--version") == 0){
            printf("%s KR qsort p.100\n", __FILE__);
            printf(usage_str, *argv);
            return 0;
        }
    }
    if (!check_arg(3, usage_str, *argv) ){
        return 1;
    }
    int         arr_len = atoi(argv[1]);
    char        gen_type = toupper(*argv[2]);
    Array      arr;
    Metric     *m = metric_create("q_sort");

    if (!inv (arr_len > 0 && (gen_type == 'A' || gen_type == 'D' || gen_type == 'R'), "only acsending (A) or descending (D) or random (R)") )
        return 2;

    switch (gen_type){
        case 'R':
            srand(time(NULL));
            arr = IArray_create(arr_len, ARRAY_RND);
        break;
        case 'A':
            arr = IArray_create(arr_len, ARRAY_ACS);    // TODO: make ACS DESC generation more useful
        break;
        case 'D':
            arr = IArray_create(arr_len, ARRAY_DESC);
        break;
    }

    g_array_rec_line = 15;
    qsort_rec(arr, 0, arr.len - 1);
    metric_print(m);
    // setup
    Array_print(arr, 500);

    logclose("...");
    return 0;
}

static void             qsort_rec(Array arr, int left, int right){
    logenter("left %d, right %d", left, right);

    int     i, last = left;
    Metric *m = metric_acq("q_sort");
    if (left >= right)
        return;

    metric_inc(m);
    int_exch(arr.iv + left, arr.iv + (left + right) / 2);

    logmsg("last %d, (%d + %d) / 2 == %d", last, left, right, (left + right) / 2);
    for (i = left + 1; i <= right; i++)
        if (arr.iv[i] < arr.iv[left])
            int_exch(arr.iv + ++last, arr.iv + i), metric_inc(m);

    metric_inc(m);
    int_exch(arr.iv + left, arr.iv + last);

    Array_fprint(logfile, arr, 100);

    qsort_rec(arr, left, last - 1);
    qsort_rec(arr, last + 1, right);
    logret(0, "...");   // TODO: log.h what if no returning value???
}

