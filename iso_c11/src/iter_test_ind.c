#include <stdio.h>
#include <stdlib.h>

#define CONCAT_EXPAND(x, y) x ## y
#define CONCAT(x, y)        CONCAT_EXPAND(x, y)
#define UNIQUE_ID(prefix)   CONCAT(prefix, __COUNTER__)

// ----------------------------- ARRAY OF POINTERS, RETURNS INDEX -------------------------------------
/*#define _foreach_arrptr_impl(i, parr, parr_name)\
        typeof_unqual(arr) parr_name = (arr);\
        _Static_assert(__builtin_classify_type(*parr) == 5, "foreach_pointer: массив должен состоять из указателей");\
        for (int i  = 0; parr_name[i] != 0; i++)

// iterate via int value
#define foreach_arrptr(i, parr) \
    _foreach_arrptr_impl(i, parr, UNIQUE_ID(_parr_))
*/
// iterate via int value
#define foreach_arrptr(i, parr)\
    _Static_assert(__builtin_classify_type(*(parr)) == 5, "Array of pointers is required");\
    for (int i = 0; parr[i] != 0; i++)

typedef struct {
    int     i;
    char    c;
} TestType;

static TestType   **getparr(TestType   **p){
    static int cnt = 0;
    printf("RUN!!!! %d\n", cnt++);
    return p;
}

int         main(void){


    // TEST1: array of pointers! That's working, OK
    TestType     arr[] = { {1,'a'}, {5, 'b'}, {11, 'c'}, {2, 'd'}, {4, 'e'}, {99, 'f'} };
    int          cnt = sizeof(arr) / sizeof(TestType), i;
    TestType   **parr = malloc( (cnt + 1) * sizeof(TestType *) );

    for (i = 0; i < cnt; i++)
        parr[i] = arr + i;
    // last 0
    parr[i] = 0;

    // try print normally, to compare
    for (int i = 0; parr[i] != 0; i++)
        printf("%d - %d:%c\n", i, parr[i]->i, parr[i]->c);

    foreach_arrptr(i, parr)
        printf("%d:%c\n", parr[i]->i, parr[i]->c);

    foreach_arrptr(i, getparr(parr) )
        printf("%d:%c\n", parr[i]->i, parr[i]->c);

    // TEST2: simple array, warning or COMPILATION ERROR must be here!!!!!
    /* int a[] = {1, 2, 3, 0};
    foreach_arrptr(i, a)
        printf("%d\n", a[i]); */

    return 0;
}

