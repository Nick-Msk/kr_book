#include <stdio.h>
#include <stdlib.h>

// to common.h
// assuming argc as checked value!!!
#define         init_checker(retval, argcnt, msg, ...)\
    if (argc < (argcnt)){\
        fprintf(stderr, (msg), ##__VA_ARGS__);\
        return ((retval));\
    }

// array.h
#define             FILLARR_ZERO        0
#define             FILLARR_RND         1
#define             FILLARR_CONST       2
#define             FILLARR_INC         3
#define             FILLARR_RNDINC      4

// fill array with diff types
void                fill_arr(int *arr, int sz, int filltype);

typedef struct {
    int     res1, res2;
} t_res;

t_res               find_common(const int *arr1, int arr1size, const int *arr2, int arr2size);

int                 main(int argc, const char *argv[]){
 
    init_checker(1, 3, "Usage: %s size1, size2\n", *argv);
    int     sz1 = atoi(argv[1]);
    int     sz2 = atoi(argv[2]);

    int     arr1[sz1];
    int     arr2[sz2];

    //TODO: generate here
    fill_arr(arr1, sz1, FILLARR_RND);
    fill_arr(arr2, sz2, FILLARR_RND);

    t_res   common_pos;
    common_pos = find_common(arr1, sz1, arr2, sz2);

    if (common_pos.res1 < 0)
        printf("Arrays have no equal elements!\n");
        // TODO: print arrays here? 
    else
        printf("arr1[%d] = %d, arr2[%d] = %d\n",
                common_pos.res1, arr1[common_pos.res1],
                common_pos.res2, arr2[common_pos.res2]);
    return 0;
}

t_res               find_common(const int *arr1, int arr1size, const int *arr2, int arr2size){
    t_res   r = {-1, -1};
    int     found = 0;
    for (int i = 0; i < arr1size && !found; i++)
        for (int j = 0; j < arr2size && !found; j++)
            if (arr1[i] == arr2[j]){
                found = 1;
                r.res1 = i;
                r.res2 = j;
            }
    return r;
}

// array.c
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

