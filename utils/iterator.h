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

#endif /* !_ITERATOR_H */
