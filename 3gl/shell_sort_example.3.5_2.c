#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>

#define             FILLARR_ZERO        0
#define             FILLARR_RND         1
#define             FILLARR_CONST       2
#define             FILLARR_INC         3
#define             FILLARR_RNDINC      4

// common.c
// fill array with diff types
void                fill_arr(int *arr, int sz, int filltype);

// print the part of array (currently it's hardcoded)
void                print_arr(const int *arr, int sz, int linesz, int maxline);

// save array into text file!
int                 save_arr(const int *arr, int sz, FILE *fw, const char *fname);

static inline
int                 save_arr_f(const int *arr, int sz, FILE *fw){
    return save_arr(arr, sz, fw, 0);
}

static inline
int                 save_arr_name(const int *arr, int sz, const char *fname){
    return save_arr(arr, sz, 0, fname);
}

// TODO: 
int                 read_arr(int *arr, int maxsz, FILE *fr);

// simple converter
long                strtoval(const char *s);

// run and collect data!  if arr == 0 then internal array will be used
int                 run_shell_sort(int *runarray, int runcount, int arrsize);

// local
long                shell_sort(int *v, int n);

int                 main(int argc, const char *argv[]){

    int         arrsz = 1024;
    int         runcnt = 100;

    if (argc > 1)
        arrsz = strtoval(argv[1]);
    if (argc > 2)
        runcnt = strtoval(argv[2]);

    printf("Array size %d with %d runs\n", arrsz, runcnt);

    int         arr[runcnt];

    //print_arr(arr, sz, 10, 8);
    run_shell_sort(arr, runcnt, arrsz);

    printf("Runs:\n");
    shell_sort(arr, runcnt); 
    print_arr(arr, runcnt, 15, 25);

    save_arr_name(arr, runcnt, "shell_runs.ser");
    return 0;
}

int                 run_shell_sort(int *runarray, int runarraycount, int arrsize){
    int     localsz = 0;
    if (runarray == 0)
        localsz = runarraycount;
    int     localrun[runarraycount];
    if (runarray == 0)
        runarray = localrun;

    int     bufarr[arrsize];

    while (runarraycount--){
        // arr is suplied or local
        fill_arr(bufarr, arrsize, FILLARR_RND);
        runarray[runarraycount] = shell_sort(bufarr, arrsize);
    }
    return 0;   // not sure what's return here
}

// common.c
long                strtoval(const char *s){
    char   *mult;
    long    val = strtol(s, &mult, 10);
    switch (tolower(*mult)){
        case 'm': 
            val *= 1000000;
        break;
        case 'k':
            val *= 1000;
        break;
    } 
    return val;
}

// common.c
void                fill_arr(int *arr, int sz, int filltype){
    sranddev(); // non ANSI
    int     inc = 0;
    switch (filltype) {
        case FILLARR_ZERO:
            for (int i = 0; i < sz; i++)
                arr[i] = 0; // TODO: make that in one function call ?
        break;
        case FILLARR_CONST:
            for (int i = 0; i < sz; i++)
                arr[i] = 3; // LOL
        break;
        case FILLARR_INC:
            for (int i = 0; i < sz; i++)
                arr[i] = (inc+= 3);  // 3 ??? LOL
        break;
        case FILLARR_RND:
            for (int i = 0; i < sz; i++)
                arr[i] = rand() % 500;
        break;
        case FILLARR_RNDINC:
            for (int i = 0; i < sz; i++)
                arr[i] = (inc += rand() % 5);
        break;
    }
}

// common.c
void            print_arr(const int *arr, int sz, int linesz, int maxline){
    for (int i = 0; i < sz && i / linesz < maxline; i++)
        printf("%8d(%3d)%c", arr[i], i, (i%linesz == linesz -1 || i == sz - 1) ? '\n' : ' ');
}

static inline
void                exchint(int *a, int *b){
    int     temp = *a;
    *a = *b;
    *b = temp;
}

long                shell_sort(int *v, int n){
    long    cnt = 0l;
    int     gap, i, j;

    for (gap = n/2; gap >0; gap /= 2){
        for (i = gap; i < n; i++)
            for (j = i - gap;  j >= 0 && v[j] > v[j + gap]; j -= gap)
                exchint(&v[j], &v[j + gap]), cnt ++;
    }
    return cnt;
}

int                 save_arr(const int *arr, int sz, FILE *fw, const char *fname){
    if (!fw && !fname){
        fprintf(stderr,  "%s: both fname and fw are empty\n", __func__);
        return -1;
    }
    if (!fw){   // fname != 0
        fw = fopen(fname, "w");
        if (!fw){
            fprintf(stderr, "%s: unable to open file %s\n", __func__, fname);
            return errno;
        }
    }

    for (int i = 0; i < sz; i++){
        // put every value on every line!! In future can be optimized
        if (fprintf(fw, "(%8d): %8d\n"
                            , i, arr[i]) < 0){
            fprintf(stderr, "%s: error during fprintf %d - error:  %d\n", __func__, i, errno);
            return errno;
        }
    }
    fclose(fw);
    return sz;  // not sure
}
