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
static int              binsearch_kr(int x, IArray arr);
// as per task 3.1 KR
static int              binsearch_typ2(int x, IArray arr);

static int              force_testing(int iter, int size);

const char *usage_str = "Usage: %s <len:int> <value:int> <type of gen A:asc T:force testing)\n";

int                     main(int argc, const char *argv[]){
    static const char *logfilename = "log/binsearch.3.1.log";
    loginit(logfilename, false, 0, "Start");    // TODO: rework that to LOG("logdir") or LOGAPPEND("logdir") or LOGSWITCH("logdir")

    if (argc > 1){
        if (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--version") == 0){
            printf("%s binsearch KR task 3.1\n", __FILE__);
            printf(usage_str, *argv);
            return 0;
        }
    }
    if (!check_arg(4, usage_str, *argv) ){
        return 1;
    }
    int     arr_len = atoi(argv[1]);
    char    gen_type = toupper(*argv[3]);
    int     val = atoi(argv[2]);

    if (!inv (arr_len > 0 && gen_type == 'A' || gen_type == 'T', "only acsending (A) or (T): force testing  generator are supported") )
        return 2;

    if (gen_type == 'A'){
        srand(time(NULL));
        IArray arr1 = IArray_create(arr_len, ARRAY_ACS);
        IArray_print(arr1, 200);

        Metric *a = metric_create("binsearch_kr");

        int res = binsearch_kr(val, arr1);
        if (res < 0)
            printf("Not found\n");
        else 
            printf("Found pos = %d\n", res);
 
        metric_print(a);

        Metric *a2 = metric_create("binsearch_typ2");
        res = binsearch_typ2(val, arr1);
        if (res < 0)
            printf("Type2: Not found\n");
        else 
            printf("Type2: Found pos = %d\n", res);

        metric_print(a2);
    } else 
        force_testing(val, arr_len);
    logclose("...");
    return 0;
}

// from KR p71
static int              binsearch_kr(int x, IArray arr){
    int l = 0, r = arr.len - 1, mid;

    Metric *m = metric_get("binsearch_kr", false);
    while (l <= r){
        mid = (l + r) / 2;
        if (x < arr.v[mid])
            r = mid - 1, metric_inc(m);
        else if (x > arr.v[mid])
            l = mid + 1, metric_inc(m);
        else {
            metric_inc(m);
            return mid;
        }
    }
    return -1;
}



// as per task 3.1 KR
static int              binsearch_typ2(int x, IArray arr){
    logenter("x=%d", x);
    int l = 0, r = arr.len - 1, mid = 0;

    Metric *m = metric_get("binsearch_typ2", false);

    while (l < r){
        mid = ((l + r) / 2);
        logmsg("mid=%d, l=%d r=%d, arr.v[mid]=%d", mid, l, r, arr.v[mid]);
        if (x > arr.v[mid])
            l = mid + 1, metric_inc(m);
        else
            r = mid, metric_inc(m);
    }
    metric_inc(m);
    logmsg("mid=%d, l=%d r=%d, arr.v[mid]=%d arr[l]=%d arr[r]=%d", 
        mid, l, r, arr.v[mid], arr.v[l], arr.v[r]);
    if (arr.v[r] == x)
        return logret(r, "ret %d", r);
    else
        return logret(-1, "Not found");
}

// make 1 iteration for every checking value
// arr MUST be filled with data
static int              iter_check(IArray arr, Metric *m1, Metric *m2){

    int     check_val;
    int     ret = 0;
    int     fnd1, fnd2;

    metric_reset(m1);
    metric_reset(m2);

    IArray_fill(arr, ARRAY_ACS);

    for (check_val = arr.v[0] - 1; check_val <= arr.v[arr.len - 1] + 1; check_val++){
        fnd1 = binsearch_typ2(check_val, arr);
        fnd2 = binsearch_kr(check_val, arr);

        if (fnd1 != fnd2){
            // TODO: raise here!
            fprintf(stderr, "!!!! %d != %d\n",  fnd1, fnd2);
            exit(3);
        }
        if (metric_getval(m1) != metric_getval(m2)){
            printf("typ2 has %d while kr method - %d\n", metric_getval(m1), metric_getval(m2));
            ret++;
        }
    }
    return ret;
}

// make a iter iteration with random  arrays 'size'
static int              force_testing(int iter, int size){
    logenter("iter %d, size %d", iter, size);

    int     res = 0;

    Metric *m1 = metric_create("binsearch_typ2");
    Metric *m2 = metric_create("binsearch_kr");

    IArray arr = IArray_create(size, ARRAY_ACS);
    for (int i = 0; i < iter; i++){
           res += iter_check(arr, m1, m2);
    }
    IArray_free(&arr);
    return logret(res, "Total diff ineration %d", res);
}

