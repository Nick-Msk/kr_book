#ifndef _ALLOC_H
#define _ALLOC_H

#include <stdio.h>
#include "common.h"

typedef long Align;

typedef union Header {
    struct {
        union Header   *ptr;
        unsigned        size;
    };
    Align       not_used;
} Header;

extern void                        *alloc(unsigned bytes);
extern void                        *alloct(unsigned bytes, unsigned size);
extern void                         afree(void *);
extern unsigned                     atotal(void);
extern int                          afprint_all(FILE *restrict out);

static inline void alloc_dummy(void *ptr, size_t cnt, double val) {
    (void)ptr; (void)cnt; (void)val;
}

#define                             afprint(out, fmt, ...) {\
                                        if (out) fprintf( (out), (fmt), ##__VA_ARGS__);\
                                        afprint_all(out);\
                                    }

#define _fill_get_t(x) _Generic((x), \
         float        : fill_float, \
         double       : fill_double,\
         default      : alloc_dummy)

#define alloc_type(cnt, TYPE) ({                                 \
    TYPE *_tmp = alloct((cnt), sizeof(TYPE));                    \
    _fill_get_t((TYPE){0})(_tmp, cnt,                           \
        _Generic((TYPE){0}, float: 0.0f, double: 0.0, default: 0)           \
    );                                                           \
    (_tmp);                                                      \
})

/*#define alloc_type(cnt, TYPE) ({                              \
    TYPE *_tmp = alloct((cnt), sizeof(TYPE));                 \
    _Generic((TYPE){0},                                       \
        float : fill_float(_tmp, cnt, 0.0f),                 \
        double: fill_double(_tmp, cnt, 0.0),                 \
        default: (void)0                                      \
    );                                                        \
    (_tmp);                                                   \
}) */



#endif /* !_ALLOC_H */
