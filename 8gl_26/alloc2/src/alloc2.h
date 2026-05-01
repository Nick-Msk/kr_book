#ifndef _ALLOC2_H
#define _ALLOC2_H

typedef long Align;

extern void                        *alloc(unsigned bytes);
extern void                        *alloct(unsigned cnts, unsigned size);
extern void                         afree(void *);
extern int                          afprint_all(FILE *restrict out);

#define                             afprint(out, fmt, ...) {\
                                        if (out) fprintf( (out), (fmt), ##__VA_ARGS__);\
                                        afprint_all(out);\
                                    }

#define _fill_get_t(x) _Generic((x), \
         float        : clean_float, \
         double       : clean_double,\
         default      : alloc_dummy)

#define alloc_type(cnt, TYPE) ({\
    TYPE *_tmp = alloct((cnt), sizeof(TYPE));\
    _fill_get_t((TYPE){0})(_tmp, cnt,);\
    (_tmp);\
})


#endif /* !_ALLOC2_H */
