/**
 * @file v64gen.h
 * @brief v64 Generator!
 */

#ifndef _V64GEN_H
#define _V64GEN_H

// ---------------------------------------------------------------------------------
// ------------------------------ Public value64 API -------------------------------
// ---------------------------------------------------------------------------------

// ----------------------------- Includes ------------------------------------------

#include <stdio.h>
#include "value64.h"
#include "v64typed.h"
#include "error.h"

// --------------------------------- CONSTANTS AND GLOBALS --------------------------

enum v64GenConstants {
                                        V64GENCOUNT = 4
};

// ---------------------------------- TYPES -----------------------------------------

typedef struct v64Gen                   v64Gen;
typedef value64                         (*v64GenFunc)(v64Gen *gen);

typedef struct v64Gen {
    v64GenFunc      fnext;
    value64_type    type;
    unsigned int    counter;
    v64typed        data[V64GENCOUNT];
} v64Gen;

// -------------------------- Registry Support API -----------------------------------

// now shortcut only for 4 registers
static inline value64         *v64GenReg0(v64Gen *gen) {
    return &gen->data[0].val;
}
#define V64GENREG0(gen) (gen->data[0])

static inline value64         *v64GenReg1(v64Gen *gen) {
    return &gen->data[1].val;
}
#define V64GENREG1(gen) (gen->data[1])

static inline value64         *v64GenReg2(v64Gen *gen) {
    return &gen->data[2].val;
}
#define V64GENREG2(gen) (gen->data[2])

static inline value64         *v64GenReg3(v64Gen *gen) {
    return &gen->data[3].val;
}
#define V64GENREG3(gen) (gen->data[3])

// ------------------------- CONSTRUCTOTS/DESTRUCTORS -------------------------------

extern v64Gen                   v64GenInit(v64GenFunc func, value64_type type, 
                                               v64typed initdata1, v64typed initdata2,
                                               v64typed initdata3, v64typed initdata4);
static inline v64Gen            v64GenInit0(v64GenFunc func, value64_type type) {
    return v64GenInit(func, type, V64TYPEDZERO(), V64TYPEDZERO(), V64TYPEDZERO(), V64TYPEDZERO());
}  
static inline v64Gen            v64GenInit1(v64GenFunc func, value64_type type, v64typed initdata1) {
    return v64GenInit(func, type, initdata1, V64TYPEDZERO(), V64TYPEDZERO(), V64TYPEDZERO());
}    
static inline v64Gen            v64GenInit2(v64GenFunc func, value64_type type, v64typed initdata1, 
                                                v64typed initdata2) {
    return v64GenInit(func, type, initdata1, initdata2, V64TYPEDZERO(), V64TYPEDZERO());
}    
static inline v64Gen            v64GenInit3(v64GenFunc func, value64_type type, v64typed initdata1, 
                                                v64typed initdata2, v64typed initdata3) {
    return v64GenInit(func, type, initdata1, initdata2, initdata3, V64TYPEDZERO());
} 
static inline v64Gen            v64GenInit4(v64GenFunc func, value64_type type, v64typed initdata1, 
                                                v64typed initdata2, v64typed initdata3, v64typed initdata4) {
    return v64GenInit(func, type, initdata1, initdata2, initdata3, initdata4);
} 
                     
static inline void              v64GenFree(v64Gen *gen) {
    if (gen) {
        for (int i = 0; i < V64GENCOUNT; i++)
            v64typedFree(&gen->data[i]);    // even if data = 0LL
    }
}
#define V64GEN_ZERO (v64Gen) { \
    .fnext = NULL, \
    .type = VALUE64_UNKNOWN, \
    .counter = 0L, \
    .data = { \
        [0] = { .val = LITERAL64_ZERO, .typ = VALUE64_UNKNOWN }, \
        [1] = { .val = LITERAL64_ZERO, .typ = VALUE64_UNKNOWN }, \
        [2] = { .val = LITERAL64_ZERO, .typ = VALUE64_UNKNOWN }, \
        [3] = { .val = LITERAL64_ZERO, .typ = VALUE64_UNKNOWN } \
    } \
}
#define V64GENFREE(gen) { v64GenFree(gen); gen = V64GEN_ZERO; }

// -------------------- ACCESS AND MODIFICATORS -------------------------------------

// ------------- GENERALLIZED ACCESS -----------------
extern value64                  v64GenNext(v64Gen *gen);
extern value64                  v64GenCurr(v64Gen *gen);

// ------------------ pre-created func V64 typed -----------------------------

// TODO: add FS/STR pattern
extern value64                  v64GenUnlimZero(v64Gen *gen);
// unchecker group. 0 -> 1 -> 2 ... INT_MAX -> INT_MIN etc...
// for bool false -> true -> false ...
// EXCEPT double. That type w/o cycling.
extern value64                  v64UncheckGenUnlimAscSeries(v64Gen *gen);
extern value64                  v64UncheckGenUnlimAscRnd(v64Gen *gen);
extern value64                  v64UncheckGenUnlimDescSeries(v64Gen *gen);
extern value64                  v64UncheckGenUnlimDescRnd(v64Gen *gen);
extern value64                  v64UncheckGenUnlimRandom(v64Gen *gen);
// check group: TODO:

// ------------------------ Wrappers for pre-created generators ---------------------

static inline v64Gen        v64GenCreatorUnlimZero(value64_type rettyp) {
    // not sure what to do, now just raiseint
    if (!value64_checktype(rettyp))
        userraiseint(ERR_UNSUPPORTED_TYPE, "Type %d not supported", rettyp);

    // REGITRSY ALLOCATION: NONE
    return v64GenInit0(v64GenUnlimZero, rettyp);       // quite simple, LOL
}

// REGITRSY ALLOCATION:
// data[0] as c-str for STR
static inline v64Gen        v64GenCreatorUnlimStrValue(const char *str) {
    return v64GenInit1(v64GenUnlimZero, VALUE64_STR, v64typedCreateStr(str));       // quite simple, LOL
}
// REGITRSY ALLOCATION:
// data[0]  value for LONG
static inline v64Gen        v64GenCreatorUnlimValue(long val) {
    return v64GenInit1(v64GenUnlimZero, VALUE64_LONG, v64typedCreateLong(val));       // quite simple, LOL
}
// REGITRSY ALLOCATION:
// data[0]  value for DBL
static inline v64Gen        v64GenCreatorUnlimDouble(double val) {
    return v64GenInit1(v64GenUnlimZero, VALUE64_DBL, v64typedCreateDbl(val));       // quite simple, LOL
}

// REGITRSY ALLOCATION:
// data[0] LONG as startpos for numeric generator
// data[1] FS as pattern for printing FS/STR 
static inline v64Gen        v64GenCreatorUnlimAscSeries(value64_type rettyp, long startpos, const char *fmt) {
    // not sure what to do, now just raiseint
    if (!value64_checktype(rettyp))
        userraiseint(ERR_UNSUPPORTED_TYPE, "Type %d not supported", rettyp);

    v64Gen tmp;
    if ( (rettyp == VALUE64_FS || rettyp == VALUE64_STR) && fmt != NULL)
        tmp = v64GenInit2(v64UncheckGenUnlimAscSeries,
                        rettyp, v64typedCreateLong(startpos), v64typedCreateStr(fmt) );
    else
        tmp = v64GenInit1(v64UncheckGenUnlimAscSeries, 
                        rettyp, v64typedCreateLong(startpos));
    return tmp;
}
// REGITRSY ALLOCATION:
// data[0] LONG as startpos for numeric random generator
// data[1] FS as pattern for printing FS/STR 
// data[2] INT as pattern for increment
static inline v64Gen        v64GenCreatorUnlimAscRnd(value64_type rettyp, long startpos, const char *fmt, int rndinc) {
    // not sure what to do, now just raiseint
    if (!value64_checktype(rettyp))
        userraiseint(ERR_UNSUPPORTED_TYPE, "Type %d not supported", rettyp);

    v64Gen tmp;
    if (rndinc < 1)
        rndinc = 1;
    if ( (rettyp == VALUE64_FS || rettyp == VALUE64_STR) && fmt != NULL)
        tmp = v64GenInit3(v64UncheckGenUnlimAscRnd,
                        rettyp, v64typedCreateLong(startpos), 
                                v64typedCreateStr(fmt), 
                                v64typedCreateInt(rndinc));
    else
        tmp = v64GenInit3(v64UncheckGenUnlimAscRnd, 
                        rettyp, v64typedCreateLong(startpos), 
                                V64TYPEDZERO(), 
                                v64typedCreateInt(rndinc));
    return tmp;
}


// ------------------------ PRINTERS/CHECKERS ---------------------------------------

extern int                      v64Techfprint(FILE *restrict out, const v64Gen *restrict gen);
static inline int               v64Techprint(const v64Gen* gen) {
    return v64Techfprint(stdout, gen);
}

// ------------------------------------ ETC. ----------------------------------------

extern bool                      v64GenValidate(FILE *restrict out, const v64Gen *restrict gen);

#endif /* !_VALUE64GEN_H */
