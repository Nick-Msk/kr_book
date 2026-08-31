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
static void             shell_sort(Array arr);

const char *usage_str = "Usage: %s <len:int> <type of gen A:asc D:desc R:random)\n";

int                     main(int argc, const char *argv[]){
    static const char *logfilename = "log/"__FILE__".log";
    loginit(logfilename, false, 0, "Start");    // TODO: rework that to LOG("logdir") or LOGAPPEND("logdir") or LOGSWITCH("logdir")

    if (argc > 1){
        if (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--version") == 0){
            printf("%s KR shellsort p.75\n", __FILE__);
            printf(usage_str, *argv);
            return 0;
        }
    }
    if (!check_arg(3, usage_str, *argv) ){
        return 1;
    }
    int         arr_len = atoi(argv[1]);
    char        gen_type = toupper(*argv[2]);
    Array       arr;
    Metric     *m = metric_create("shell_sort");

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
    shell_sort(arr);
    metric_print(m);
    //
    Array_print(arr, 500);

    logclose("...");
    return 0;
}

static void             shell_sort(Array arr){
    int     gap, i, j;
    Metric *m = metric_acq("shell_sort");
    for (gap = arr.len / 2; gap > 0; gap /= 2)
        for (i = gap; i < arr.len; i++){
            for (j = i - gap; j >= 0 && arr.iv[j] > arr.iv[j + gap]; j-= gap){
                 int_exch(arr.iv + j, arr.iv + j + gap);
                 metric_inc(m);
            }
            /* IArray_print(arr, 500); // tech print
            printf("--------------------------------------------\n"); */
        }
}

