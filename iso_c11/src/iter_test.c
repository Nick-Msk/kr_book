#include <stdio.h>
#include <stdlib.h>

//#define              foreach_arr(i, arr) for (typeof_unqual (*arr) *iter = arr, i = *iter; *iter != 0; i = *++iter)

#define CONCAT_EXPAND(x, y) x ## y
#define CONCAT(x, y)        CONCAT_EXPAND(x, y)
#define UNIQUE_ID(prefix)   CONCAT(prefix, __COUNTER__)

#define foreach_pointer_impl(i, arr, iter_name) \
    for (typeof_unqual(*arr)* iter_name = (arr), i = *iter_name; \
         *iter_name != 0; \
         i = *++iter_name)

#define foreach_pointer(i, arr) \
    foreach_pointer_impl(i, arr, UNIQUE_ID(_iter_))

typedef struct {
    int     i;
    char    c;
} TestType;

int         main(void){


    // TEST1: array of pointers! That's working, OK
    TestType     arr[] = { {1,'a'}, {5, 'b'}, {11, 'c'}, {2, 'd'}, {4, 'e'} };
    int          cnt = sizeof(arr) / sizeof(TestType), i;
    TestType   **parr = malloc( (cnt + 1) * sizeof(TestType *) );

    for (i = 0; i < cnt; i++)
        parr[i] = arr + i;
    // last 0
    parr[i] = 0;

    // try print normally, to compare
    for (int i = 0; parr[i] != 0; i++)
        printf("%d - %d:%c\n", i, parr[i]->i, parr[i]->c);

    foreach_pointer(item, parr)
        printf("%d:%c\n", item->i, item->c);

    // TEST2: simple array, warning or error must be here
    int a[] = {1, 2, 3, 0};
    foreach_pointer(y, a)
        printf("%d\n", y);

    return 0;
}
