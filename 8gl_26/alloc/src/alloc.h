#ifndef _ALLOC_H
#define _ALLOC_H

#include <stdio.h>

typedef long Align;

typedef union Header {
    struct {
        union Header   *ptr;
        unsigned        size;
    };
    Align       not_used;
} Header;

extern void                        *alloc(unsigned bytes);
extern void                         afree(void *);
extern unsigned                     atotal(void);
extern int                          afprint_all(FILE *restrict out);
#define                             afprint(out, fmt, ...) {\
                                        if (out) fprintf( (out), (fmt), ##__VA_ARGS__);\
                                        afprint_all(out);\
                                    }

#endif /* !_ALLOC_H */
