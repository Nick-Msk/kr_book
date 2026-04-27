#ifndef _ALLOC_H
#define _ALLOC_H

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

#endif /* !_ALLOC_H */
