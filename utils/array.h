#ifndef _ARRAY_H
#define _ARRAY_H

#include <stdio.h>

#include "common.h"
#include "log.h"
#include "checker.h"

// ---------------------------------------------------------------------------------
// --------------------------- Public Array API ------------------------------------
// ---------------------------------------------------------------------------------

// ----------- CONSTANTS AND GLOBALS ---------------

// array, but not IArray, because common for int and double
typedef enum ArrayFillType{
    ARRAY_NONE      = 0,
    ARRAY_DESC,
    ARRAY_ASC,
    ARRAY_RND,
    ARRAY_ZERO,
    ARRAY_ASC_SERIES,
    ARRAY_DESC_SERIES
} ArrayFillType;

typedef enum ArrayType{
    ARRAY_DOUBLE    = 0x1,
    ARRAY_INT       = 0x2,
    ARRAY_LONG      = 0x3,
    ARRAY_POINTER   = 0x4,
    ARRAY_ERROR     = 0x100
} ArrayType;

static inline const char        *ArrayTypeName(ArrayType t){
    switch (t & (ARRAY_DOUBLE | ARRAY_INT | ARRAY_POINTER | ARRAY_ERROR) ){
        CASE_RETURN(ARRAY_DOUBLE);
        CASE_RETURN(ARRAY_INT);
        CASE_RETURN(ARRAY_LONG);
        CASE_RETURN(ARRAY_POINTER);
        CASE_RETURN(ARRAY_ERROR);
        default: return "";
    }
}

static inline const char        *ArrayFillTypeName(ArrayFillType t){
    switch (t){
        CASE_RETURN(ARRAY_NONE);
        CASE_RETURN(ARRAY_DESC);
        CASE_RETURN(ARRAY_ASC);
        CASE_RETURN(ARRAY_RND);
        CASE_RETURN(ARRAY_ZERO);
        CASE_RETURN(ARRAY_ASC_SERIES);
        CASE_RETURN(ARRAY_DESC_SERIES);
        default: return "";
    }
}

extern int              g_array_rec_line;
extern const char      *g_custom_print_line;
// ------------------- TYPES -----------------------

typedef struct {
    int     len;
    int     sz; // total size, > len + 1
    int     flags; // ARRAY_DOUBLE || ARRAY_INT || ARRAY_POINTER
    union {
        void    *v;
        int     *iv;
        long    *lv;
        double  *dv;
        void   **pv;    // pointer array
    };
} Array;

// ------------- CONSTRUCTOTS/DESTRUCTORS --------------

// init
#define                         IArray_init(...) (Array){.len = 0, .sz = 0, .iv = 0, .flags = ARRAY_INT, __VA_ARGS__}
#define                         LArray_init(...) (Array){.len = 0, .sz = 0, .lv = 0, .flags = ARRAY_LONG, __VA_ARGS__}
#define                         DArray_init(...) (Array){.len = 0, .sz = 0, .dv = 0, .flags = ARRAY_DOUBLE, __VA_ARGS__}
#define                         PArray_init(...) (Array){.len = 0, .sz = 0, .iv = 0, .flags = ARRAY_POINTER, __VA_ARGS__}
#define                         Array_init(...)  (Array){.len = 0, .sz = 0, .iv = 0, .flags = 0, __VA_ARGS__}
#define                         Arrayfree(x)({ Array_free(&(x)); (x).iv = 0; })

// --------------- CREATE  and fill --------------------
extern Array                    Array_create(int cnt, ArrayFillType filltyp, ArrayType typ);

extern void                     Array_free(Array *val);

static inline Array             IArray_create(int cnt, ArrayFillType typ){
    return Array_create(cnt, typ, ARRAY_INT);
}
static inline Array             LArray_create(int cnt, ArrayFillType typ){
    return Array_create(cnt, typ, ARRAY_LONG);
}
static inline Array             DArray_create(int cnt, ArrayFillType typ){
    return Array_create(cnt, typ, ARRAY_DOUBLE);
}
static inline Array             PArray_create(int cnt, ArrayFillType typ){
    return Array_create(cnt, typ, ARRAY_POINTER);
}


// -------------- ACCESS AND MODIFICATION --------------

static inline int               Array_gettype(Array a){
    return a.flags & 0xFF;
}

static inline bool              Array_isint(Array a){
    return Array_gettype(a) == ARRAY_INT;
}
static inline bool              Array_islong(Array a){
    return Array_gettype(a) == ARRAY_LONG;
}
static inline bool              Array_isdouble(Array a){
    return Array_gettype(a) == ARRAY_DOUBLE;
}
static inline bool              Array_ispointer(Array a){
    return Array_gettype(a) == ARRAY_POINTER;
}
static inline bool              Array_iserror(Array a){
    return a.flags & ARRAY_ERROR;
}

static inline Array             Array_seterror(Array a){
    a.flags |= ARRAY_ERROR;
    return a;
}
static inline bool              Array_isvalid(Array a){
    return ( ( !(a.flags & ARRAY_ERROR) && a.flags &
            (ARRAY_INT | ARRAY_LONG | ARRAY_DOUBLE | ARRAY_POINTER) ) > 0) && a.sz >= a.len && a.len >= 0 && a.iv != 0;
}

static inline int               Arraylen(Array a){
    return a.len;
}

static inline int               ArrayGetcnt(Array a){
    invraise(Array_ispointer(a), "Applicable only for pointers ARRAY_POINTER %d", ARRAY_POINTER);
    int cnt = 0;
    for (int i = 0; i < a.len; i++)
        cnt += a.pv[i] != 0;
    return logsimpleret(cnt, "Total valuable elem %d", cnt);
}

static inline int               Arraysz(Array a){
    return a.sz;
}

extern int                      Array_fill(Array arr, ArrayFillType typ);
extern int                      Array_fillrange(Array a, ArrayFillType typ, int from, int to);
extern Array                    Array_shrink(Array arr, int newsz);
extern Array                    Array_increase(Array arr, int newcnt);

extern void                     Array_shuffle(Array arr);
extern void                     Array_qsort(Array arr, ArrayFillType ord);

//#define                         Array_apply(arr, condition, action)
#define Array_foreach(arr, elem, idx, block) do { \
    typeof_unqual(arr) _arr = (arr); \
    if (Array_isint(_arr)) { \
        for (int idx = 0; idx < _arr.len; idx++) { \
            int *elem = &_arr.iv[idx]; \
            block \
        } \
    } else if (Array_islong(_arr)) { \
        for (int idx = 0; idx < _arr.len; idx++) { \
            long *elem = &_arr.lv[idx]; \
            block \
        } \
    } else if (Array_isdouble(_arr)) { \
        for (int idx = 0; idx < _arr.len; idx++) { \
            double *elem = &_arr.dv[idx]; \
            block \
        } \
    } else if (Array_ispointer(_arr)) { \
        for (int idx = 0; idx < _arr.len; idx++) { \
            void **elem = &_arr.pv[idx]; \
            block \
        } \
    } \
} while(0)

// ----------------- PRINTERS ----------------------

extern int                      Array_fprint(FILE *f, Array val, int limit);

static inline int               Array_print(Array val, int limit){
    return Array_fprint(stdout, val, limit);
}

extern long                     Array_save(Array arr, const char *fname);

extern Array                    Array_load(const char *fname);

// save only values by delimeter
extern long                     Array_savevalues(Array arr, const char *fname, char delim);

// ------------------ ETC. -------------------------

#endif /* !_ARRAY_H */

