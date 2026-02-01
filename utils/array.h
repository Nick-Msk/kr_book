#ifndef _ARRAY_H
#define _ARRAY_H

#include <stdio.h>

#include "common.h"
#include "log.h"

// ---------------------------------------------------------------------------------
// --------------------------- Public <Skeleton> API ----------------------------------
// ---------------------------------------------------------------------------------

// ----------- CONSTANTS AND GLOBALS ---------------

// array, but not IArray, because common for int and double
typedef enum ArrayFillType{
    ARRAY_NONE      = 0,
    ARRAY_DESC,
    ARRAY_ACS,
    ARRAY_RND,
    ARRAY_ZERO
} ArrayFillType;

typedef enum ArrayType{
    ARRAY_DOUBLE    = 0x1,
    ARRAY_INT       = 0x2,
    ARRAY_ERROR     = 0x100
} ArrayType;

static inline const char        *ArrayFillTypeName(ArrayFillType t){
    switch (t){
        CASE_RETURN(ARRAY_NONE);
        CASE_RETURN(ARRAY_DESC);
        CASE_RETURN(ARRAY_ACS);
        CASE_RETURN(ARRAY_RND);
        CASE_RETURN(ARRAY_ZERO);
        default: return "";
    }
}

extern int              g_array_rec_line;
extern const char      *g_custom_print_line;
// ------------------- TYPES -----------------------

typedef struct {
    int     len;
    int     sz; // total size, > len + 1
    int     flags; // ARRAY_DOUBLE || ARRAY_INT
    union {
        int    *iv;
        double *dv;
    };
} Array;

/*
typedef struct {
    int     len;
    int     sz; // total size, > len + 1
    double  *v;
} DArray; */

// ------------- CONSTRUCTOTS/DESTRUCTORS ----------

// init
#define                         IArray_init(...) (Array){.len = 0, .sz = 0, .iv = 0, .flags = ARRAY_INT, __VA_ARGS__}
#define                         DArray_init(...) (Array){.len = 0, .sz = 0, .dv = 0, .flags = ARRAY_DOUBLE, __VA_ARGS__}
#define                         Array_init(...)  (Array){.len = 0, .sz = 0, .iv = 0, .flags = 0, __VA_ARGS__}
#define                         Arrayfree(x){ Array_free(&(x)); (x).iv = 0; }

// CREATE  and fill with method
extern Array                    Array_create(int cnt, ArrayFillType filltyp, ArrayType typ);

extern void                     Array_free(Array *val);

static inline Array             IArray_create(int cnt, ArrayFillType typ){
    return Array_create(cnt, typ, ARRAY_INT);
}
static inline Array             DArray_create(int cnt, ArrayFillType typ){
    return Array_create(cnt, typ, ARRAY_DOUBLE);
}

extern Array                    Array_shrink(Array arr, int newsz);

// -------------- ACCESS AND MODIFICATION ----------

static inline bool              Array_isint(Array a){
    return a.flags & ARRAY_INT;
}

static inline bool              Array_isdouble(Array a){
    return a.flags & ARRAY_DOUBLE;
}

static inline bool              Array_iserror(Array a){
    return a.flags & ARRAY_ERROR;
}

static inline Array             Array_seterror(Array a){
    a.flags |= ARRAY_ERROR;
    return a;
}

static inline bool              Array_isvalid(Array a){
    return ( ( !(a.flags & ARRAY_ERROR) && a.flags & (ARRAY_INT | ARRAY_DOUBLE) ) > 0) && a.sz >= a.len && a.len >= 0 && a.iv != 0;
}

extern void                     Array_fill(Array a, ArrayFillType typ);

// ----------------- PRINTERS ----------------------

extern int                      Array_fprint(FILE *f, Array val, int limit);

static inline int               Array_print(Array val, int limit){
    return Array_fprint(stdout, val, limit);
}

extern long                     Array_save(Array arr, const char *fname);

extern Array                    Array_load(const char *fname);

/*
// ----------------- Double -----------------------

// CREATE  and fill with method for double
extern DArray                   DArray_create(int cnt, ArrayFillType typ);

// the same: TODO: create a macro Array_free()
extern void                     DArray_free(DArray *val);

// -------------- ACCESS AND MODIFICATION ----------

extern void                     DArray_fill(DArray a, ArrayFillType typ);

// ----------------- PRINTERS ----------------------

extern int                      DArray_fprint(FILE *f, DArray val, int limit);

static inline int               DArray_print(DArray val, int limit){
    return DArray_fprint(stdout, val, limit);
} */
// ------------------ ETC. -------------------------

#endif /* !_ARRAY_H */

