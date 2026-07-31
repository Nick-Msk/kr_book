#ifndef _ARRAY_H
#define _ARRAY_H

#include <stdio.h>
#include <limits.h>
#include <time.h>
#include <stdlib.h>
#include <float.h>

#include "common.h"
#include "log.h"
#include "checker.h"
#include "value64.h"    // for Value64
#include "error.h"

// ---------------------------------------------------------------------------------
// --------------------------- Public Array API ------------------------------------
// ---------------------------------------------------------------------------------

// ----------- CONSTANTS AND GLOBALS ---------------

// array, but not IArray, because common for int and double
typedef enum ArrayFillType{
    ARRAY_FILLTYPE_NONE      = 0,
    ARRAY_FILLTYPE_DESC,
    ARRAY_FILLTYPE_ASC,
    ARRAY_FILLTYPE_RND,
    ARRAY_FILLTYPE_ZERO,
    ARRAY_FILLTYPE_ASC_SERIES,
    ARRAY_FILLTYPE_DESC_SERIES
} ArrayFillType;

typedef enum ArrayType{
    ARRAY_UNKNOWN   = 0x0,
    ARRAY_DOUBLE    = 0x1,
    ARRAY_INT       = 0x2,
    ARRAY_LONG      = 0x3,
    ARRAY_POINTER   = 0x4,
    ARRAY_V64       = 0x5,      // value64 port
    ARRAY_ERROR     = 0x100
} ArrayType;

static inline const char        *ArrayTypeName(ArrayType t){
    switch (t & (ARRAY_DOUBLE | ARRAY_INT | ARRAY_POINTER | ARRAY_ERROR) ){
        CASE_RETURN(ARRAY_DOUBLE);
        CASE_RETURN(ARRAY_INT);
        CASE_RETURN(ARRAY_LONG);
        CASE_RETURN(ARRAY_POINTER);
        CASE_RETURN(ARRAY_ERROR);
        CASE_RETURN(ARRAY_V64);
        default: return "";
    }
}

static inline const char        *ArrayFillTypeName(ArrayFillType t){
    switch (t){
        CASE_RETURN(ARRAY_FILLTYPE_NONE);
        CASE_RETURN(ARRAY_FILLTYPE_DESC);
        CASE_RETURN(ARRAY_FILLTYPE_ASC);
        CASE_RETURN(ARRAY_FILLTYPE_RND);
        CASE_RETURN(ARRAY_FILLTYPE_ZERO);
        CASE_RETURN(ARRAY_FILLTYPE_ASC_SERIES);
        CASE_RETURN(ARRAY_FILLTYPE_DESC_SERIES);
        default: return "";
    }
}

extern int              g_array_rec_line;
extern const char      *g_custom_print_line;
// ------------------- TYPES -----------------------

typedef struct {
    int             len;
    int             sz; // total size, > len + 1
    int             flags; // ARRAY_DOUBLE || ARRAY_INT || ARRAY_POINTER || ARRAY_V64
    value64_type    v64type;        // applicable only for ARRAY_V64
    union {
        void    *v;
        int     *iv;
        long    *lv;
        double  *dv;
        void   **pv;    // pointer array
        value64 *v64;   // container type
    };
} Array;
// condition func
typedef         bool (*Array_cond)(Array arr, int pos);
// prosessing func
typedef         void (*Array_proc)(Array arr, int pos);

// ------------- CONSTRUCTOTS/DESTRUCTORS --------------

// init
#define                         IArray_init(...) (Array){.len = 0, .sz = 0, .iv = 0, .flags = ARRAY_INT, __VA_ARGS__}
#define                         LArray_init(...) (Array){.len = 0, .sz = 0, .lv = 0, .flags = ARRAY_LONG, __VA_ARGS__}
#define                         DArray_init(...) (Array){.len = 0, .sz = 0, .dv = 0, .flags = ARRAY_DOUBLE, __VA_ARGS__}
#define                         PArray_init(...) (Array){.len = 0, .sz = 0, .iv = 0, .flags = ARRAY_POINTER, __VA_ARGS__}
#define                         V64Array_init(...) (Array){.len = 0, .sz = 0, .iv = 0, .flags = ARRAY_V64, __VA_ARGS__}
#define                         Array_init(...)  (Array){.len = 0, .sz = 0, .iv = 0, .flags = 0, __VA_ARGS__}
#define                         Arrayfree(x)({ Array_free(&(x)); (x).iv = 0; })

// --------------- CREATE  and fill --------------------
extern Array                    Array_create(int cnt, ArrayFillType filltyp, ArrayType typ, value64_type vt);

extern void                     Array_free(Array *val);

static inline Array             IArray_create(int cnt, ArrayFillType typ){
    return Array_create(cnt, typ, ARRAY_INT, VALUE64_UNKNOWN);
}
static inline Array             LArray_create(int cnt, ArrayFillType typ){
    return Array_create(cnt, typ, ARRAY_LONG, VALUE64_UNKNOWN);
}
static inline Array             DArray_create(int cnt, ArrayFillType typ){
    return Array_create(cnt, typ, ARRAY_DOUBLE, VALUE64_UNKNOWN);
}
static inline Array             PArray_create(int cnt, ArrayFillType typ){
    return Array_create(cnt, typ, ARRAY_POINTER, VALUE64_UNKNOWN);
}
static inline Array             V64Array_create(int cnt, ArrayFillType typ, value64_type vt){
    return Array_create(cnt, typ, ARRAY_V64, vt);
}


// -------------- ACCESS AND MODIFICATION --------------
static inline bool              Array_isv64(Array a);

static inline int               Array_gettype(Array a) {
    return a.flags & 0xFF;
}

static inline const char       *ArrayGettypeName(Array a) {
    return ArrayFillTypeName(Array_gettype(a) );
}

static inline const char       *ArrayGetV64typeName(Array a) {
    if (Array_isv64(a))
        return value64_typename(a.v64type);
    else
        return "Not V64 type";
}

static inline ArrayType         ArrayGetV64mappedType(Array a) {
    ArrayType res = Array_gettype(a);
    if (res == ARRAY_V64) {
        // TODO: probably use a mapping table
        switch (a.v64type) {
            case VALUE64_INT:
                res = ARRAY_INT;
                break;
            case VALUE64_LNG:
                res = ARRAY_LONG;
                break;
            case VALUE64_DBL:
                res = ARRAY_DOUBLE;
                break;
            case VALUE64_PTR:
                res = ARRAY_POINTER;
                break;
            default:
                // TODO: error
                res = ARRAY_UNKNOWN;
                break;
        }
    }
    return res;
}
/// @brief check if array is INT
/// @param a array
/// @return true if INT
static inline bool              Array_isint(Array a){
    return Array_gettype(a) == ARRAY_INT;
}
/// @brief check if array is LONG
/// @param a array
/// @return true if LONG
static inline bool              Array_islong(Array a){
    return Array_gettype(a) == ARRAY_LONG;
}
/// @brief check if array is DBL
/// @param a array
/// @return true if DBL
static inline bool              Array_isdouble(Array a){
    return Array_gettype(a) == ARRAY_DOUBLE;
}
/// @brief check if array is PTR
/// @param a array
/// @return true if PTR
static inline bool              Array_ispointer(Array a){
    return Array_gettype(a) == ARRAY_POINTER;
}
/// @brief check if array is VALUE64
/// @param a array
/// @return true if VALUE64
static inline bool              Array_isv64(Array a){
    return Array_gettype(a) == ARRAY_V64;
}
/// @brief check if array is in error state /* NOT USED */
/// @param a array
/// @return true if error state 
static inline bool              Array_iserror(Array a){
    return a.flags & ARRAY_ERROR;
}
/// @brief set error state to array /* NOT USED */
/// @param a array
/// @return array
static inline Array             Array_seterror(Array a){
    a.flags |= ARRAY_ERROR;
    return a;
}
/// @brief check if array is valie
/// @param a array
/// @return true if ok 
static inline bool              Array_isvalid(Array a){
    return ( ( !(a.flags & ARRAY_ERROR) && a.flags &
            (ARRAY_INT | ARRAY_LONG | ARRAY_DOUBLE | ARRAY_POINTER | ARRAY_V64
            ) ) > 0) && a.sz >= a.len && a.len >= 0 && a.iv != 0;
}
/// @brief get array length (count of formatted values)
/// @param a array
/// @return count of formatted values
static inline int               Arraylen(Array a){
    return a.len;
}
/// @brief get array size (total allocated values)
/// @param a array
/// @return total allocated values
static inline int               Arraysz(Array a){
    return a.sz;
}
/// @brief get count of non-null pointers
/// @param a array
/// @return count of non-null pointers
static inline int               ArrayGetcnt(Array a){
    invraise(Array_ispointer(a), "Applicable only for pointers ARRAY_POINTER %d", ARRAY_POINTER);
    int cnt = 0;
    for (int i = 0; i < a.len; i++)
        cnt += a.pv[i] != 0;
    return logsimpleret(cnt, "Total valuable elem %d", cnt);
}


extern int                      Array_fill(Array arr, ArrayFillType typ);
extern int                      Array_fillrange(Array a, ArrayFillType typ, int from, int to);
extern Array                    Array_shrink(Array arr, int newsz);
extern Array                    Array_increase(Array arr, int newcnt);

extern void                     Array_shuffle(Array arr);
extern void                     Array_qsort(Array arr, ArrayFillType ord);
// ---------------------------- binary searchers --------------------------------
// int
extern int                      ArrayBsearchIntCommon(Array arr, int val, bool acs);
static inline int               ArrayBsearchInt(Array arr, int val) {
    return ArrayBsearchIntCommon(arr, val, true);
}
static inline int               ArrayBsearchIntrev(Array arr, int val) {
    return ArrayBsearchIntCommon(arr, val, false);
}
// long
extern int                      ArrayBsearchLongCommon(Array arr, long val, bool acs);
static inline int               ArrayBsearchLong(Array arr, long val) {
    return ArrayBsearchLongCommon(arr, val, true);
}
static inline int               ArrayBsearchLongRev(Array arr, long val) {
    return ArrayBsearchLongCommon(arr, val, false);
}
// double
extern int                      ArrayBsearchDblCommon(Array arr, double val, bool acs);
static inline int               ArrayBsearchDbl(Array arr, double val) {
    return ArrayBsearchDblCommon(arr, val, true);
}
static inline int               ArrayBsearchDblRev(Array arr, double val) {
    return ArrayBsearchDblCommon(arr, val, false);
}
// V64
extern int                      ArrayBsearchV64Common(Array arr, value64 val, bool acs);
static inline int               ArrayBsearchV64(Array arr, value64 val) {
    return ArrayBsearchV64Common(arr, val, true);
}
static inline int               ArrayBsearchV64Rev(Array arr, value64 val) {
    return ArrayBsearchV64Common(arr, val, false);
}
// -------------------------------------- foreach ---------------------------------------
// if condition is 0-ptr == ALL
extern int                      Array_foreach_proc(Array arr, Array_cond cond, Array_proc func);
// if condition is 0-ptr == ALL
// TODO:
extern int                      Array_foreach_rev_proc(Array arr, Array_cond cond, Array_proc func);

//#define                       Array_apply(arr, condition, action)

// Общий макрос: p is arr.iv, len is arr.len
#define                         _Array_foreach_gen(p, len, elem) \
    for (typeof_unqual(*(p)) *_begin_ = (p), *(elem) = _begin_; \
         (elem) < _begin_ + (len); \
         ++(elem))
// reverse
 #define                         _Array_foreach_rev_gen(p, len, elem) \
    for (typeof_unqual(*(p)) *_begin_ = (p) + (len) - 1, *(elem) = _begin_; \
         (elem) >= _begin_ ; \
         --(elem))        

// Публичные однобуквенные макросы
#define IArray_foreach(arr, elem)   _Array_foreach_gen((arr).iv, (arr).len, elem)
#define LArray_foreach(arr, elem)   _Array_foreach_gen((arr).lv, (arr).len, elem)
#define DArray_foreach(arr, elem)   _Array_foreach_gen((arr).dv, (arr).len, elem)
#define PArray_foreach(arr, elem)   _Array_foreach_gen((arr).pv, (arr).len, elem)
#define V64Array_foreach(arr, elem) _Array_foreach_gen((arr).v64, (arr).len, elem)

// ----------------- PRINTERS ----------------------

extern int                      Array_fprint(FILE *f, Array val, int limit);

static inline int               Array_print(Array val, int limit){
    return Array_fprint(stdout, val, limit);
}

extern long                     Array_save(Array arr, const char *fname);
extern long                     ArrayFSave(FILE *out, Array arr);
extern Array                    Array_load(const char *fname);
extern Array                    ArrayFLoad(FILE *in);

// save only values by delimeter
extern long                     Array_savevalues(Array arr, const char *fname, char delim);

// ------------------ ETC. -------------------------

#endif /* !_ARRAY_H */

