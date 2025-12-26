#include <stdio.h>
#include <stdlib.h>

#define         FILLARR_ZERO        0
#define         FILLARR_RND         1
#define         FILLARR_CONST       2
#define         FILLARR_INC         3
#define         FILLARR_RNDINC      4

void            fill_arr(int *arr, int sz, int filltype);

void            print_arr(const int *arr, int sz, int linesz, int maxline);

int             binary_search(int val, const int *arr, int sz);

int             main(int argc, const char *argv[]){

    if (argc < 3){
        fprintf(stderr, "Usage: %s cnt\n", *argv);
        return 1;
    }
    int     sz = strtol(argv[1], 0, 10);
    int     val = strtol(argv[2], 0, 10);

    if (sz <= 0){
        fprintf(stderr, "Usage: sz = %d, must be positive\n", sz);
        return 1;
    }
    int     arr[sz];
    //int     arr[] = {1, 2, 3, 5, 7, 7, 7, 7, 8, 12,
    //   16, 17, 21, 24, 28, 32, 36, 38, 42, 43};
    fill_arr(arr, sz, FILLARR_RNDINC);
    print_arr(arr, sz, 10, 5);
    int     res = binary_search(val, arr, sz);
    if (res < 0)
        printf("%d isn''t found\n", val);
    else
        printf("Position of %d is %d\n", val, res);

    return 0;
}

void            fill_arr(int *arr, int sz, int filltype){
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
                arr[i] = rand();
        break;
        case FILLARR_RNDINC:
            for (int i = 0; i < sz; i++)
                arr[i] = (inc += rand() % 5);
        break;
    }
}

void            print_arr(const int *arr, int sz, int linesz, int maxline){
    for (int i = 0; i < sz && i / linesz < maxline; i++)
        printf("%8d(%3d)%c", arr[i], i, (i%linesz == linesz -1 || i == sz - 1) ? '\n' : ' ');
}

int             binary_search(int val, const int *arr, int sz){
    int     low = 0, high = sz - 1, mid;

    while (low <= high && val != arr[mid]){
        mid  = (low + high) / 2;
        //fprintf(stderr, "arr[%d] = %d\n", mid, arr[mid]);
        if (val < arr[mid])
            high = mid - 1;
        else
            low = mid + 1;
    }
    //fprintf(stderr, "END:  arr[%d] = %d\n", mid, arr[mid]);
    return arr[mid] == val? mid: -1;
}

/*
int             binary_search(int val, const int *arr, int sz){
    int     low = 0, high = sz - 1, mid;

    while (low <= high){
        mid  = (low + high) / 2;
        if (val < arr[mid])
            high = mid - 1;
        else if (val > arr[mid])
            low = mid + 1;
        else
            return mid;
    }
    return -1;
} */

