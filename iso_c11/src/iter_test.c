#include <stdio.h>
#include <stdlib.h>

#define              foreach_arr(i, arr) for (typeof_unqual (*arr) *iter = arr, i = *iter; *iter != 0; i = *++iter)

// with solved const issue
/*#define foreach_arr(i, arr) \
    for (typeof(arr) _iter_arr_ = (arr), \
         typeof_unqual(*_iter_arr_) i = *_iter_arr_; \
         *_iter_arr_ != NULL; \
         i = *++_iter_arr_)
*/

int         main(void){
    int     arr[] = {1, 5, 11, 2, 4};
    int     sz = sizeof(arr) / sizeof(int), i;
    int   **parr = malloc( (sz + 1) * sizeof(int *) );

    for (i = 0; i < (int) (sizeof(arr) / sizeof(int) ); i++)
        parr[i] = arr + i;
    // last 0
    parr[i] = 0;

    // try print normally
    for (int i = 0; parr[i] != 0; i++)
        printf("%d - %d\n", i, *parr[i] );

    foreach_arr(item, parr)
        printf("%d\n", *item);

    return 0;
}
