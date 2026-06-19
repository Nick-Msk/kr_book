#include <stdio.h>
#include <stdlib.h>

#include "guard.h"
#include "log.h"
#include "common.h"
#include "checker.h"

static int          pregen_array[] = {4, -3, -1, 6, 3, 11, -7, -1, 8, 24, 11};

static int          print_tripters(const int *arr, int cnt);
static int          fprint_array(FILE *restrict out, const int *restrict arr, int cnt);
static int         *get_array(int sz);

int                 main(int argc, const char *argv[]){
    logsimpleinit("Start");

    if (!check_arg(2, "Usage: %s arr length\n", *argv) )
        return 1;

    int         len = atoi(argv[1]);
    if (len < 3 || len >= COUNT(pregen_array) ){
        fprintf(stderr, "Len (%d) mMust be between 3 and %d\n", len, COUNT(pregen_array) );
        return 2;
    }

    int        *arr = get_array(len);
    fprint_array(stdout, arr, len);

    return logret(0, "Done");
}

static int         *get_array(int sz){
    if (sz <= COUNT(pregen_array) )
        return pregen_array;
    else
        return NULL;
}

static int          fprint_array(FILE *restrict out, const int *restrict arr, int cnt){
    int  outcnt = 0;
    for (int i = 0; i < cnt; i++){
        outcnt += fprintf(out, "%d\t", arr[i]);
        if ( (i + 1) % 10 == 0)
            outcnt += fprintf(out, "\n");
    }
    outcnt += fprintf(out, "\n");
    return outcnt;
}
