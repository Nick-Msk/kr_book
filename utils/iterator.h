#ifndef _ITERATOR_H
#define _ITERATOR_H

#include <limits.h>

#define foreachint(var, ...) \
    for (int *_iter = (int[]){__VA_ARGS__, INT_MAX}, var = *_iter; \
         var != INT_MAX; \
         var = *++_iter)

#define foreachlong(var, ...) \
    for (long *_iter = (int[]){__VA_ARGS__, LONG_MAX}, var = *_iter; \
         var != LONG_MAX; \
         var = *++_iter)

#define foreachstring(var, ...) \
    for (const char **_iter = (int[]){__VA_ARGS__, 0x0}, var = *_iter; \
         var != 0; \
         var = *++_iter)

#endif /* !_ITERATOR_H */
