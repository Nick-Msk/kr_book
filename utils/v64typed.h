#ifndef _V64TYPED_H
#define _V64TYPED_H

// -----------------------------------------------------------------------------------
// ------------------------------ Public typed v64 API -------------------------------
// -----------------------------------------------------------------------------------

// ------------------------------- Includes ------------------------------------------

#include <stdio.h>

#include "value64.h"
#include "error.h"
#include "checker.h"

// ---------------------------------- CONSTANTS AND GLOBALS --------------------------

// ------------------------------------ TYPES ----------------------------------------

/** @brief Wrapper over value64 (value + type) */
typedef struct {
    value64      val;   ///< само значение
    value64_type typ;   ///< его тип (VALUE64_INT, VALUE64_FS, ...)
} v64typed;

// ---------------------------- CONSTRUCTORS / DESTRUCTORS ---------------------------

// COMMON ZERO INIT
#define V64TYPEDZERO(...) (v64typed) { .val = LITERAL64_ZERO, .typ = VALUE64_UNKNOWN, __VA_ARGS__ };

// PER TYPE
#define V64TYPEDINT(val)    (v64typed){ .val = LITERAL64_INT(val),  .typ = VALUE64_INT }
#define V64TYPEDLONG(val)   (v64typed){ .val = LITERAL64_LONG(val), .typ = VALUE64_LONG }
#define V64TYPEDCHAR(val)   (v64typed){ .val = LITERAL64_LONG(val), .typ = VALUE64_CHR }
#define V64TYPEDBOOL(val)   (v64typed){ .val = LITERAL64_LONG(val), .typ = VALUE64_BOOL }
#define V64TYPEDDBL(val)    (v64typed){ .val = LITERAL64_DBL(val),  .typ = VALUE64_DBL }
#define V64TYPEDPTR(val)    (v64typed){ .val = LITERAL64_PTR(val),  .typ = VALUE64_PTR }
#define V64TYPEDSTR(val)    (v64typed){ .val = LITERAL64_STR(val),  .typ = VALUE64_STR }
// dangerous one! Probably 'll be removed
#define V64TYPEDFS(val)     (v64typed){ .val = LITERAL64_PFS(val),  .typ = VALUE64_FS }


static inline v64typed              v64typedCommon(value64 val, value64_type typ) {
    return V64TYPEDZERO(.val = val, .typ = typ);
}
static inline v64typed              v64typedInt(int x) {
    return v64typedCommon(LITERAL64_INT(x), VALUE64_INT);
}
static inline v64typed              v64typedLong(long x) {
    return (v64typed){ .val = LITERAL64_LONG(x), .typ = VALUE64_LONG };
}
static inline v64typed              v64typedDbl(double x) {
    return (v64typed){ .val = LITERAL64_DBL(x), .typ = VALUE64_DBL };
}
static inline v64typed              v64typedStr(const char *s) {
    // Внимание: строка не копируется, ожидается, что она существует всё время использования массива
    return (v64typed){ .val = LITERAL64_STR(s), .typ = VALUE64_STR };
}
static inline v64typed              v64typedFs(fs *s) {
    return (v64typed){ .val = LITERAL64_PFS(s), .typ = VALUE64_FS };
}
static inline v64typed              v64typedUnk(void) {
    return (v64typed){ .val = LITERAL64_ZERO, .typ = VALUE64_UNKNOWN };
}
static inline v64typed              v64typedClone(v64typed v) {
    return (v64typed) { .val = value64_clone(v.val, v.typ), .typ = v.typ };
}
//  move constructor
static inline v64typed              v64typedMove(v64typed *v) {
    v64typed   res = { .val = value64_move(&v->val, v->typ), .typ = v->typ};
    v->typ = VALUE64_UNKNOWN;
    return res;
}


// -------------------- ACCESS AND MODIFICATORS -------------------------------------

// nvl for STR
static inline const char*
v64typedNvlStr(v64typed tv, const char *default_fmt)
{
    if ( (tv.typ == VALUE64_STR) && !strisempty(value64_str(tv.val) ) )
            return value64_str(tv.val);
    else 
        return default_fmt;
}
// nvl for FS
static inline const char*
value64GenNvlFs(v64typed tv, const char *restrict default_fmt) {

    if ( tv.typ == VALUE64_FS && !fs_isnull(value64_fs(tv.val) ) )
            return fs_str(value64_fs(tv.val) );
    else
        return logsimpleerr(default_fmt, "Incorrent type %d %s, FS expected", tv.typ, value64_typename(tv.typ) );
}
// nvl for INT, works as type cast!
static inline int
value64GenNvlInt(v64typed tv, int default_val) {
    int     res = 0;
    switch (tv.typ) {
        case VALUE64_INT:
            res = value64_int(tv.val);
            break;
        case VALUE64_LONG:
            res = value64_long(tv.val);
            break;
        case VALUE64_ULONG:
            res = value64_ulong(tv.val);
            break;
        case VALUE64_CHR:
            res = value64_char(tv.val);
            break;
        case VALUE64_BOOL:
            res = value64_bool(tv.val);
            break;
        case VALUE64_DBL:
            res = value64_dbl(tv.val);
            break;
        default:
            return logsimpleerr(res, "Incorrent type %d %s, numeric or bool are expected", tv.typ, value64_typename(tv.typ) );       
    }
    return res ? res : default_val;
}

// note only int value to add, but generally it's ok
// no OF checking version
static inline bool
v64typedAdd(v64typed *tv, int value) {
    switch (tv->typ) {
        case VALUE64_INT:
            tv->val.ival += value;
            break;
        case VALUE64_LONG:
            tv->val.lval += (long) value;
            break;
        case VALUE64_ULONG:
            tv->val.ulval += (unsigned long) value;
            break;
        case VALUE64_CHR:
            tv->val.cval += (unsigned char) value;
            break;
        case VALUE64_DBL:
            tv->val.dval += (double) value;
            break;
        default:
            return userraise(false, ERR_UNSUPPORTED_TYPE, 
                "Unsupported typ %d %s", tv->typ, value64_typename(tv->typ) );
    }
    return true;
}

static inline bool
v64typedBoolNegative(v64typed *tv) {
    switch (tv->typ) {
        case VALUE64_BOOL:
            tv->val.bval = !tv->val.bval;
            break;
        default:
            return userraise(false, ERR_UNSUPPORTED_TYPE, 
                "Unsupported typ %d %s", tv->typ, value64_typename(tv->typ) );
    }
    return true;
}

// ------------------------ PRINTERS/CHECKERS ---------------------------------------

extern int                      v64typedTechfprint(FILE *restrict out, v64typed tval, const char *restrict name);
static inline int               v64typedTechprint(v64typed tval, const char * name) {
    return v64typedTechfprint(stdout, tval, name);
}

#define V64TYPED_TECHFPRINT(out, val, typ)   v64typedTechfprint( (out), (val), #val)
#define V64TYPED_TECHPRINT(val, typ)         v64typedTechprint((val), #val)

// ------------------------------------ ETC. ----------------------------------------
extern bool                     v64typedValidate(FILE *out, v64typed tval);

#endif /* _V64TYPED_H */
