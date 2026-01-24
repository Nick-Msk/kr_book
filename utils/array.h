#ifndef ARRAY_H
#define ARRAY_H

#include <stdio.h>

#include "common.h"
#include "log.h"

// ---------------------------------------------------------------------------------
// --------------------------- Public <Skeleton> API ----------------------------------
// ---------------------------------------------------------------------------------

// ----------- CONSTANTS AND GLOBALS ---------------

// array, but not IArray, because common for int and double
typedef enum ArrayFillType{
    ARRAY_NONE = 0,
    ARRAY_DESC,
    ARRAY_ACS,
    ARRAY_RND,
    ARRAY_ZERO
} ArrayFillType;

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

// ------------------- TYPES -----------------------

typedef struct {
    int     len;
    int     sz; // total size, > len + 1
    int    *v;
} IArray;

typedef struct {
    int     len;
    int     sz; // total size, > len + 1
    double  *v;
} DArray;

// ------------- CONSTRUCTOTS/DESTRUCTORS ----------

// init
#define                         IArray_init(...) (IArray){.len = 0, .sz = 0, .v = 0}

// CREATE  and fill with method
extern IArray                   IArray_create(int cnt, ArrayFillType typ);

extern void                     IArray_free(IArray *val);

// -------------- ACCESS AND MODIFICATION ----------

extern void                     IArray_fill(IArray a, ArrayFillType typ);

// ----------------- PRINTERS ----------------------

extern int                      IArray_fprint(FILE *f, IArray val, int limit);

static inline int               IArray_print(IArray val, int limit){
    return IArray_fprint(stdout, val, limit);
}

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
}
// ------------------ ETC. -------------------------

#endif /* !ARRAY_H */

