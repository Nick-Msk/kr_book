#ifndef _VALUE64_H
#define _VALUE64_H

// ---------------------------------------------------------------------------------
// --------------------------- Public Faststring API -------------------------------
// ---------------------------------------------------------------------------------

// ----------------------------- Includes ------------------------------------------

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>
//
#include "bool.h"
#include "log.h"
#include "common.h"
#include "error.h"
#include "checker.h"
#include "fs.h"
#include "numeric_ops.h"

// --------------------------------- CONSTANTS AND GLOBALS --------------------------

// ---------------------------------- TYPES -----------------------------------------

// ANY type must be inside uint64_t
typedef union value64 {
        int                 ival;
        long                lval;
        double              dval;
        char               *sval;
        fs                 *fsval;
        void               *pval;
        uint64_t            u64;    // for hash
} value64;

_Static_assert(sizeof(value64) == sizeof(uint64_t),
               "value64 must be exactly as uint64_t");

typedef enum value64_type {
    VALUE64_UKNOWN = 0,
    VALUE64_INT = 1,
    VALUE64_LNG,
    VALUE64_DBL,
    VALUE64_FS,
    VALUE64_PTR,
    VALUE64_STR,
    VALUE64_TYPE_COUNT
} value64_type;

typedef struct {
    const char  *name;
    size_t       size;
    bool         is_valid;
    const char  *type_desc;
} value64_typeinfo;

extern const                        value64_typeinfo* value64_info_get(value64_type typ);

static inline bool                  value64_checktype(value64_type typ) {
    return value64_info_get(typ) != NULL;
}

static inline const char            *value64_typename(value64_type t) {
    const value64_typeinfo* info = value64_info_get(t);
    if (!info)
        return userraise(NULL, ERR_UNSUPPORTED_TYPE, "Type %d not supported", t);
    return info->name;
}
extern  value64_type                 value64_gettype(const char *str);
// only zero for now
#define                 VALUE64_ZERO      (value64) {.u64 = 0L }


/*#define                 VALUE64_INT(val)  (value64) {.u64 = 0L, .ival = val }
#define                 VALUE64_LONG(val) (value64) {.u64 = 0L, .lval = val }
#define                 VALUE64_DBL(val)  (value64) {.u64 = 0L, .dval = val }
#define                 VALUE64_PTR(val)  (value64) {.u64 = 0L, .pval = val }
// pointer copy!!!
#define                 VALUE64_STR(val)  (value64) {.u64 = 0L, .sval = val }
// local version
#define                 VALUE64_FSVALUE(val) (value64) {.u64 = 0L, .fsval = &(val) }
// pointer version TODO: not sure, commented for now

#define                 VALUE64_FSPVALUE(pval) (hset_value) {.fsval = fs_heapcreate(pval) }
//move version
#define                 VALUE64_FSMOVE(val)    (hset_value) {.fsval = fs_moveto_heap(val) } */




// ------------------------- CONSTRUCTOTS/DESTRUCTORS -------------------------------

// the part of mass creation API
// create value from pointer, value64 constructor ANY type, MOVE semantic
extern value64                      value64_pcopy_move(void *p, value64_type typ, bool move);
// copy, for array operations
static inline value64               value64_pinit(const void *p, value64_type typ){
    return value64_pcopy_move( (void *) p, typ, false);
}
// create value from pointer, value64 constructor ANY type, MOVE semantic
static inline value64               value64_pmove(void *p, value64_type typ){
    return value64_pcopy_move(p, typ, true);
}
// copy logic
static inline value64               value64_createint(int val){
    value64 tmp = VALUE64_ZERO;
    tmp.ival = val;
    return tmp;
}
static inline value64               value64_createlong(long lval){
    value64 tmp = VALUE64_ZERO;
    tmp.lval = lval;
    return tmp;
}
static inline value64               value64_createdbl(double dval){
    value64 tmp = VALUE64_ZERO;
    tmp.dval = dval;
    return tmp;
}
static inline value64               value64_createptr(void *pval){
    value64 tmp = VALUE64_ZERO;
    tmp.pval = pval;
    return tmp;
}
// TODO: movestr?
static inline value64               value64_createstr(const char *sval){
    if (!sval)
        userraiseint(ERR_NULLABLE_PTR, "Null pointer");
    value64 tmp = VALUE64_ZERO;
    if ( (tmp.sval = strdup(sval) ) == NULL)
        userraiseint(ERR_UNABLE_ALLOCATE, "Unable to dup string");
    return tmp;
}
static inline value64               value64_createfs(const fs *fsval){
    if (!fsval || !fsval->v)
        userraiseint(ERR_NULLABLE_PTR, "Null pointer fs %p or fs->v %p", fsval, fsval ? fsval->v: NULL);
    value64 tmp = VALUE64_ZERO;
    if ( (tmp.fsval = fs_heapcreate(fsval) ) == NULL)
        userraiseint(ERR_UNABLE_ALLOCATE, "Unable to dup fs");
    return tmp;
}
// just switch over the types!
static inline value64               value64_clone(value64 source, value64_type typ){
    switch (typ){
        case VALUE64_INT:
            return value64_createint(source.ival);
        case VALUE64_LNG:
            return value64_createlong(source.lval);
        case VALUE64_DBL:
            return value64_createdbl(source.dval);
        case VALUE64_FS:
            return value64_createfs(source.fsval);
        case VALUE64_PTR:
            return value64_createptr(source.pval);
        case VALUE64_STR:
            return value64_createstr(source.sval);
        default:
            return VALUE64_ZERO;
    }
}
// move constructor
static inline value64               value64_movefs(const fs *fsval){
    value64 tmp = VALUE64_ZERO;
    if ( (tmp.fsval = fs_moveto_heap( (fs *) fsval) ) == NULL)
        userraiseint(ERR_UNABLE_ALLOCATE, "Unable to dup fs");
    return tmp;
}
static inline void                  value64_free(value64 v, value64_type typ){
    switch (typ){
        case VALUE64_STR:
            free(v.sval);   // even if NULL
        break;
        case VALUE64_FS:
            fs_free(v.fsval);   // even if NULL
        break;
        default:
        break;
    }
}
static inline void                  value64_freefs(value64 v){
    return value64_free(v, VALUE64_FS);
}
static inline void                  value64_freestr(value64 v){
    return value64_free(v, VALUE64_STR);
}

// -------------------- ACCESS AND MODIFICATORS -------------------------------------
// just get
static inline int                   value64_int(value64 v){
    return v.ival;
}
static inline long                  value64_long(value64 v){
    return v.lval;
}
static inline double                value64_dbl(value64 v){
    return v.dval;
}
static inline char                 *value64_str(value64 v){
    return v.sval;
}
static inline void                 *value64_ptr(value64 v){
    return v.pval;
}
static inline fs                   *value64_fs(value64 v){
    return v.fsval;
}

// move to EXISTING object, thart is NOT a constructor
// move switcher to EXISTING object
static inline value64              *value64_move(value64 *restrict target, value64 *restrict source, value64_type typ){
    invraisecode(target && source,  ERR_NULLABLE_PTR, "Null pointers %p %p", target, source);

    switch (typ){
        case VALUE64_FS:    // note: this's NOT the same as value64_movefs!
            target->fsval = fs_moveto_heap(source->fsval);  // no need to null source, fs_moveto_heap'll do that
            if (!target->fsval)
                userraiseint(ERR_UNABLE_ALLOCATE, "Unable to alloc new fs body");
        break;
        case VALUE64_DBL:
            target->dval = source->dval;
            source->dval = 0.0;
        break;
        default:    // ALL others type even VALUE64_STR follows the same logic!
            target->u64 = source->u64;
            source->u64 = 0L;   // u64 cover all types
        break;
    }
    return target;
}
static inline value64              *value64_move_int(value64 *restrict target, value64 *restrict source){
    return value64_move(target, source, VALUE64_INT);
}
static inline value64              *value64_move_long(value64 *restrict target, value64 *restrict source){
    return value64_move(target, source, VALUE64_LNG);
}
static inline value64              *value64_move_dbl(value64 *restrict target, value64 *restrict source){
    return value64_move(target, source, VALUE64_DBL);
}
static inline value64              *value64_move_str(value64 *restrict target, value64 *restrict source){
    return value64_move(target, source, VALUE64_STR);
}
static inline value64              *value64_move_ptr(value64 *restrict target, value64 *restrict source){
    return value64_move(target, source, VALUE64_PTR);
}
static inline value64              *value64_move_fs(value64 *restrict target, value64 *restrict source){
    return value64_move(target, source, VALUE64_FS);
}

extern unsigned long               value64_lhash(value64 value, value64_type typ);

extern value64                     value64_convert(value64 v, value64_type from, value64_type to);
extern value64                     value64_convert_move(value64 *source, value64_type from, value64_type to);

// SQL in low level
extern bool                        value64_in   (value64 val, value64_type typ, const value64 *arr, int sz);
extern bool                        value64_notin(value64 val, value64_type typ, const value64 *arr, int sz);

// ------------------------ PRINTERS/CHECKERS ---------------------------------------

static inline void                  value64_fprint(FILE *restrict out, const char *restrict msg, value64 val, value64_type typ){
    if (out){
        if (msg)
            fprintf(out, "%s ", msg);
        switch (typ){
            case VALUE64_INT:
                fprintf(out, "%d", val.ival);
            break;
            case VALUE64_LNG:
                fprintf(out, "%ld", val.lval);
            break;
            case VALUE64_DBL:
                fprintf(out, "%lf", val.dval);
            break;
            case VALUE64_PTR:
                fprintf(out, "%p", val.pval);
            break;
            case VALUE64_STR:
                fprintf(out, "%s", val.sval);
            break;
            case VALUE64_FS:
                fs_fprint(out, val.fsval, 0);
            break;
            default:
                fprintf(out, "Unsupported %d!\n", typ);
            break;
        }
    }
}
static inline void          value64_log(value64 val, value64_type typ){
    value64_fprint(logfile, 0, val, typ);
}
static inline void          value64_print(value64 val, value64_type typ){
    value64_fprint(stdout, 0, val, typ);
}

// --------------------------------- SERIALIZATION ----------------------------------

// ------------------------------------ ETC. ----------------------------------------

#endif /* !_VALUE64_H */

