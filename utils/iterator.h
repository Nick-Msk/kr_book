#ifndef _ITERATOR_H
#define _ITERATOR_H

#include <limits.h>

#define foreachint(var, ...) \
    for (int *_iter = (int[]){__VA_ARGS__, INT_MAX}, var = *_iter; \
         var != INT_MAX; \
         var = *++_iter)

#define foreachuint(var, ...) \
    for (unsigned *_iter = (unsigned[]){__VA_ARGS__, UINT_MAX}, var = *_iter; \
         var != UINT_MAX; \
         var = *++_iter)

#define foreachlong(var, ...) \
    for (long *_iter = (long[]){__VA_ARGS__, LONG_MAX}, var = *_iter; \
         var != LONG_MAX; \
         var = *++_iter)

#define foreachulong(var, ...) \
    for (unsigned long *_iter = (unsigned long[]){__VA_ARGS__, ULONG_MAX}, var = *_iter; \
         var != ULONG_MAX; \
         var = *++_iter)

#define foreachstring(var, ...) \
    for (const char **_iter = (const char *[]){__VA_ARGS__, 0x0}, var = *_iter; \
         var != 0; \
         var = *++_iter)

#define CONCAT_EXPAND(x, y) x ## y
#define CONCAT(x, y)        CONCAT_EXPAND(x, y)
#define UNIQUE_ID(prefix)   CONCAT(prefix, __COUNTER__)

#define _foreach_pointer_impl(i, arr, iter_name) \
        _Static_assert(__builtin_classify_type(*arr) == 5, \
                       "foreach_pointer: массив должен состоять из указателей"); \
        for (typeof_unqual(*arr)* iter_name = (arr), i = *iter_name; \
                  *iter_name != 0;\
                  i = *++iter_name)

#define _foreach_pointer_named(i, arr, arr_name) \
    typeof_unqual(arr) arr_name = (arr); \
    _foreach_pointer_impl(i, arr_name, UNIQUE_ID(_iter_))

#define foreach_pointer(i, arr) \
    _foreach_pointer_named(i, arr, UNIQUE_ID(_arr_))

#endif /* !_ITERATOR_H */
