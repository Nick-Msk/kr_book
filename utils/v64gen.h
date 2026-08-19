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
#include "getword.h"

// --------------------------------- CONSTANTS AND GLOBALS --------------------------

enum v64GenConstants {
                                        V64GENCOUNT = 4
};

// ---------------------------------- TYPES -----------------------------------------


typedef struct v64Gen                   v64Gen;

typedef value64                         (*v64GenFunc)(v64Gen *gen);
typedef bool                            (*v64genUpdateStreamFunc) (v64Gen *gen, long amount);
typedef unsigned long                   (*v64GenGerRemainingFunc) (v64Gen *gen);
typedef value64                         (*v64GenFinalizerFunc)(v64Gen *gen);

typedef struct v64Gen {
    v64GenFunc              fnext;
    value64_type            type;
    unsigned int            counter;
    v64genUpdateStreamFunc  updater;
    v64GenGerRemainingFunc  remaining;
    v64GenFinalizerFunc     finalizer;
    v64typed                data[V64GENCOUNT];
} v64Gen;

// -------------------------- Registry Support API -----------------------------------

// now shortcut only for 4 registers
static inline value64              *v64GenReg0(v64Gen *gen) {
    return &gen->data[0].val;
}
#define V64GENREG0(gen) (gen->data[0])
#define V64GENREGVAL0(gen) V64GENREG0(gen).val

static inline value64              *v64GenReg1(v64Gen *gen) {
    return &gen->data[1].val;
}
#define V64GENREG1(gen) (gen->data[1])
#define V64GENREGVAL1(gen) V64GENREG1(gen).val

static inline value64              *v64GenReg2(v64Gen *gen) {
    return &gen->data[2].val;
}
#define V64GENREG2(gen) (gen->data[2])
#define V64GENREGVAL2(gen) V64GENREG2(gen).val

static inline value64              *v64GenReg3(v64Gen *gen) {
    return &gen->data[3].val;
}
#define V64GENREG3(gen) (gen->data[3])
#define V64GENREGVAL3(gen) V64GENREG3(gen).val

// ---------------------------- Unility -------------------------------
// internal, no-checking
static inline void              _v64GenFixRndinc(int *rndinc) {
    if (*rndinc < 1) {
        logsimple("rndinc = %d, set to 1", *rndinc);
        *rndinc = 1;
    }
}

// ------------------------- CONSTRUCTOTS/DESTRUCTORS -------------------------------

extern v64Gen                       v64GenInit(v64GenFunc func, value64_type type, 
                                               v64typed initdata1, v64typed initdata2,
                                               v64typed initdata3, v64typed initdata4);
static inline v64Gen                v64GenInit0(v64GenFunc func, value64_type type) {
    return v64GenInit(func, type, V64TYPEDZERO(), V64TYPEDZERO(), V64TYPEDZERO(), V64TYPEDZERO());
}  
static inline v64Gen                v64GenInit1(v64GenFunc func, value64_type type, v64typed initdata1) {
    return v64GenInit(func, type, initdata1, V64TYPEDZERO(), V64TYPEDZERO(), V64TYPEDZERO());
}    
static inline v64Gen                v64GenInit2(v64GenFunc func, value64_type type, v64typed initdata1, 
                                                v64typed initdata2) {
    return v64GenInit(func, type, initdata1, initdata2, V64TYPEDZERO(), V64TYPEDZERO());
}    
static inline v64Gen                v64GenInit3(v64GenFunc func, value64_type type, v64typed initdata1, 
                                                v64typed initdata2, v64typed initdata3) {
    return v64GenInit(func, type, initdata1, initdata2, initdata3, V64TYPEDZERO());
} 
static inline v64Gen                v64GenInit4(v64GenFunc func, value64_type type, v64typed initdata1, 
                                                v64typed initdata2, v64typed initdata3, v64typed initdata4) {
    return v64GenInit(func, type, initdata1, initdata2, initdata3, initdata4);
} 
                     
static inline void                  v64GenFree(v64Gen *gen) {
    if (gen) {
        for (int i = 0; i < V64GENCOUNT; i++)
            v64typedFree(&gen->data[i]);    // even if data = 0LL
    }
}

static inline v64Gen                v64GenZero(void) {
    return (v64Gen) {
        .fnext      = NULL,
        .type       = VALUE64_UNKNOWN,
        .counter    = 0U,
        .updater  = NULL,
        .data = {
            [0] = { .val = LITERAL64_ZERO, .typ = VALUE64_UNKNOWN },
            [1] = { .val = LITERAL64_ZERO, .typ = VALUE64_UNKNOWN },
            [2] = { .val = LITERAL64_ZERO, .typ = VALUE64_UNKNOWN },
            [3] = { .val = LITERAL64_ZERO, .typ = VALUE64_UNKNOWN }
        }
    };
}

#define V64GENFREE(gen) { v64GenFree(gen); gen = v64GenZero(); }

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
extern value64                  v64GenUnlimRandom(v64Gen *gen);
// check group: TODO:

// source (Ds or c-str or FILE *) group
extern value64                  v64GenStringToChar(v64Gen *gen);
extern value64                  v64GenFSToChar(v64Gen *gen);
/**
 * @brief Parse fs from fs, dividev by newline
 * @note
 * @return: value64/fs
 * @note
 * data[0] – PTR to source fs (non-owning)
 * data[1] – LONG current read position
 */
extern value64                  v64GenFSToFsByNewline(v64Gen *gen);
/**
 * @brief Parse fs from fs, dividev by newline
 * @note
 * @return: value64/str
 * @note
 * data[0] – PTR to source fs (non-owning)
 * data[1] – LONG current read position
 */
extern value64                  v64GenFSToStrByNewline(v64Gen *gen);
/**
 * @brief File stream generator
 *
 * data[0] – PTR на FILE* (невладеющий)
 * data[1] – ULONG remained count
 */
extern value64                  v64GenFileToChar(v64Gen *gen);
/**
 * @brief File stream generator
 *
 * data[0] – PTR на FILE* (невладеющий)
 * data[1] – ULONG remained count
 * data[2] = fs as buf (owner)
 */
extern value64                  v64GenFileToFsByNewline(v64Gen *gen);
/**
 * @brief File stream generator
 *
 * data[0] – PTR на FILE* (невладеющий)
 * data[1] – ULONG remained count
 * data[2] = fs as buf (owner)
 */
extern value64                  v64GenFileToStrByNewline(v64Gen *gen);

// ------------------------ Wrappers for pre-created generators ---------------------
// ----------------------------------------------------------------------------------

static inline v64Gen            v64GenCreatorUnlimZero(value64_type rettyp) {
    // not sure what to do, now just raiseint
    if (!value64_checktype(rettyp))
        userraiseint(ERR_UNSUPPORTED_TYPE, "Type %d not supported", rettyp);

    // REGITRSY ALLOCATION: NONE
    return v64GenInit0(v64GenUnlimZero, rettyp);       // quite simple, LOL
}

// REGITRSY ALLOCATION:
// data[0] as c-str for STR
static inline v64Gen            v64GenCreatorUnlimStrValue(const char *str) {
    return v64GenInit1(v64GenUnlimZero, VALUE64_STR, v64typedCreateStr(str));       // quite simple, LOL
}
// REGITRSY ALLOCATION:
// data[0]  value for LONG
static inline v64Gen            v64GenCreatorUnlimValue(long val) {
    return v64GenInit1(v64GenUnlimZero, VALUE64_LONG, v64typedCreateLong(val));       // quite simple, LOL
}
// REGITRSY ALLOCATION:
// data[0]  value for DBL
static inline v64Gen            v64GenCreatorUnlimDouble(double val) {
    return v64GenInit1(v64GenUnlimZero, VALUE64_DBL, v64typedCreateDbl(val));       // quite simple, LOL
}

// -------------------------------------------- ACS SERIES ------------------------------------------------

// REGITRSY ALLOCATION:
// data[0] LONG as startpos for numeric generator
// data[1] STR as pattern for printing str ONLY
static inline v64Gen            v64GenCreatorUnlimAscFsSeries(long startpos, const char *fmt) {
    v64Gen tmp;
    if (fmt != NULL)
        tmp = v64GenInit2(v64UncheckGenUnlimAscSeries,
                        VALUE64_FS, v64typedCreateLong(startpos), v64typedCreateStr(fmt) );
    else
        tmp = v64GenInit1(v64UncheckGenUnlimAscSeries, 
                        VALUE64_FS, v64typedCreateLong(startpos));
    return tmp;
}
// REGITRSY ALLOCATION:
// data[0] LONG as startpos for numeric generator
// data[1] STR as pattern for printing STR ONLY
static inline v64Gen            v64GenCreatorUnlimAscStrSeries(long startpos, const char *fmt) {
    v64Gen tmp;
    if (fmt != NULL)
        tmp = v64GenInit2(v64UncheckGenUnlimAscSeries,
                        VALUE64_STR, v64typedCreateLong(startpos), v64typedCreateStr(fmt) );
    else
        tmp = v64GenInit1(v64UncheckGenUnlimAscSeries, 
                        VALUE64_STR, v64typedCreateLong(startpos));
    return tmp;
}
// REGITRSY ALLOCATION:
// data[0] <PARAM VT TYPE> as startpos for numeric asc generator
// usage: v64GenCreatorUnlimAscStrSeries(v64typedInt(100));
static inline v64Gen            v64GenCreatorUnlimAscSeries(v64typed vt) {
    value64_type rettyp = vt.typ;     // that is it!!!

    if (int_notin(rettyp, VALUE64_BOOL, VALUE64_CHR, VALUE64_INT, VALUE64_LONG, VALUE64_ULONG, VALUE64_DBL))
        userraiseint(ERR_UNSUPPORTED_TYPE, "Type %d %s not supported by %s", 
                rettyp, value64_typename(rettyp), __func__);

    v64Gen res = v64GenInit1(v64UncheckGenUnlimAscSeries, rettyp, vt);
    return res;
}


// -------------------------------------------- DESC SERIES ------------------------------------------------

// REGITRSY ALLOCATION:
// data[0] LONG as startpos for numeric desc generator
// data[1] STR as pattern for printing str ONLY
static inline v64Gen            v64GenCreatorUnlimDescFsSeries(long startpos, const char *fmt) {
    v64Gen tmp;
    if (fmt != NULL)
        tmp = v64GenInit2(v64UncheckGenUnlimDescSeries,
                        VALUE64_FS, v64typedCreateLong(startpos), v64typedCreateStr(fmt) );
    else
        tmp = v64GenInit1(v64UncheckGenUnlimDescSeries, 
                        VALUE64_FS, v64typedCreateLong(startpos));
    return tmp;
}
// REGITRSY ALLOCATION:
// data[0] LONG as startpos for numeric desc generator
// data[1] STR as pattern for printing STR ONLY
static inline v64Gen            v64GenCreatorUnlimDescStrSeries(long startpos, const char *fmt) {
    v64Gen tmp;
    if (fmt != NULL)
        tmp = v64GenInit2(v64UncheckGenUnlimDescSeries,
                        VALUE64_STR, v64typedCreateLong(startpos), v64typedCreateStr(fmt) );
    else
        tmp = v64GenInit1(v64UncheckGenUnlimDescSeries, 
                        VALUE64_STR, v64typedCreateLong(startpos));
    return tmp;
}
// REGITRSY ALLOCATION:
// data[0] <PARAM VT TYPE> as startpos for numeric asc generator
static inline v64Gen        v64GenCreatorUnlimDescSeries(v64typed vt) {
    value64_type rettyp = vt.typ;     // that is it!!!

    if (int_notin(rettyp, VALUE64_BOOL, VALUE64_CHR, VALUE64_INT, VALUE64_LONG, VALUE64_ULONG, VALUE64_DBL))
        userraiseint(ERR_UNSUPPORTED_TYPE, "Type %d %s not supported by %s", 
                rettyp, value64_typename(rettyp), __func__);

    v64Gen res = v64GenInit1(v64UncheckGenUnlimDescSeries, rettyp, vt);
    return res;
}

// ------------------------------------ ASC RANDOM SERIES -----------------------------------------------

// REGITRSY ALLOCATION:
// data[0] LONG as startpos for numeric random generator
// data[1] STR as pattern for printing
// data[2] INT as pattern for increment
static inline v64Gen            v64GenCreatorUnlimAscFsRnd(long startpos, const char *fmt, int rndinc) {
    v64Gen tmp;
    
    _v64GenFixRndinc(&rndinc);
    if (fmt != NULL)
        tmp = v64GenInit3(v64UncheckGenUnlimAscRnd,
                        VALUE64_FS, v64typedCreateLong(startpos), 
                                v64typedCreateStr(fmt), 
                                v64typedCreateInt(rndinc));
    else
        tmp = v64GenInit3(v64UncheckGenUnlimAscRnd, 
                        VALUE64_FS, v64typedCreateLong(startpos), 
                                V64TYPEDZERO(), 
                                v64typedCreateInt(rndinc));
    return tmp;
}

// REGITRSY ALLOCATION:
// data[0] LONG as startpos for numeric random generator
// data[1] STR as pattern for printing
// data[2] INT as pattern for increment
static inline v64Gen            v64GenCreatorUnlimAscStrRnd(long startpos, const char *fmt, int rndinc) {
    v64Gen tmp;

    _v64GenFixRndinc(&rndinc);
    if (fmt != NULL)
        tmp = v64GenInit3(v64UncheckGenUnlimAscRnd,
                        VALUE64_STR, v64typedCreateLong(startpos), 
                                v64typedCreateStr(fmt), 
                                v64typedCreateInt(rndinc));
    else
        tmp = v64GenInit3(v64UncheckGenUnlimAscRnd, 
                        VALUE64_STR, v64typedCreateLong(startpos), 
                                V64TYPEDZERO(), 
                                v64typedCreateInt(rndinc));
    return tmp;
}

// REGITRSY ALLOCATION:
// data[0] <PARAM VT TYPE> as startpos for numeric asc generator
// data[2] INT as pattern for increment
static inline v64Gen            v64GenCreatorUnlimAscRnd(v64typed vt, int rndinc) {
    value64_type rettyp = vt.typ;     // that is it!!!

    if (int_notin(rettyp, VALUE64_BOOL, VALUE64_CHR, VALUE64_INT, VALUE64_LONG, VALUE64_ULONG, VALUE64_DBL))
        userraiseint(ERR_UNSUPPORTED_TYPE, "Type %d %s not supported by %s", 
                rettyp, value64_typename(rettyp), __func__);

     _v64GenFixRndinc(&rndinc);   
    v64Gen res = v64GenInit3(v64UncheckGenUnlimAscRnd, 
                            rettyp, vt, 
                            V64TYPEDZERO(), 
                            v64typedCreateInt(rndinc));
    return res;
}

// ------------------------------------ DESC RANDOM SERIES -----------------------------------------------

// REGITRSY ALLOCATION:
// data[0] LONG as startpos for numeric desc random generator
// data[1] STR as pattern for printing
// data[2] INT as pattern for increment
static inline v64Gen            v64GenCreatorUnlimDescFsRnd(long startpos, const char *fmt, int rndinc) {
    v64Gen tmp;

     _v64GenFixRndinc(&rndinc);
    if (fmt != NULL)
        tmp = v64GenInit3(v64UncheckGenUnlimDescRnd,
                        VALUE64_FS, v64typedCreateLong(startpos), 
                                v64typedCreateStr(fmt), 
                                v64typedCreateInt(rndinc));
    else
        tmp = v64GenInit3(v64UncheckGenUnlimDescRnd, 
                        VALUE64_FS, v64typedCreateLong(startpos), 
                                V64TYPEDZERO(), 
                                v64typedCreateInt(rndinc));
    return tmp;
}

// REGITRSY ALLOCATION:
// data[0] LONG as startpos for numeric desc random generator
// data[1] STR as pattern for printing
// data[2] INT as pattern for increment
static inline v64Gen            v64GenCreatorUnlimDescStrRnd(long startpos, const char *fmt, int rndinc) {
    v64Gen tmp;

     _v64GenFixRndinc(&rndinc);
    if (fmt != NULL)
        tmp = v64GenInit3(v64UncheckGenUnlimDescRnd,
                        VALUE64_STR, v64typedCreateLong(startpos), 
                                v64typedCreateStr(fmt), 
                                v64typedCreateInt(rndinc));
    else
        tmp = v64GenInit3(v64UncheckGenUnlimDescRnd, 
                        VALUE64_STR, v64typedCreateLong(startpos), 
                                V64TYPEDZERO(), 
                                v64typedCreateInt(rndinc));
    return tmp;
}

// REGITRSY ALLOCATION:
// data[0] <PARAM VT TYPE> as startpos for numeric desc generator
// data[2] INT as pattern for increment
static inline v64Gen            v64GenCreatorUnlimDescRnd(v64typed vt, int rndinc) {
    value64_type rettyp = vt.typ;     // that is it!!!

    if (int_notin(rettyp, VALUE64_BOOL, VALUE64_CHR, VALUE64_INT, VALUE64_LONG, VALUE64_ULONG, VALUE64_DBL))
        userraiseint(ERR_UNSUPPORTED_TYPE, "Type %d %s not supported by %s", 
                rettyp, value64_typename(rettyp), __func__);

     _v64GenFixRndinc(&rndinc);
    v64Gen res = v64GenInit3(v64UncheckGenUnlimDescRnd, rettyp, vt, V64TYPEDZERO(), v64typedCreateInt(rndinc));
    return res;
}

// ------------------------------------ JUST RANDOM -----------------------------------------------
// REGITRSY ALLOCATION:
// data[0] INT as lim for rnsint
static inline v64Gen            v64GenCreatorUnlimRnd(value64_type rettyp, int rndinc) {
    if (int_notin(rettyp, VALUE64_BOOL, VALUE64_CHR, VALUE64_INT, VALUE64_LONG, VALUE64_ULONG, VALUE64_DBL))
        userraiseint(ERR_UNSUPPORTED_TYPE, "Type %d %s not supported by %s", 
                rettyp, value64_typename(rettyp), __func__);

    _v64GenFixRndinc(&rndinc);
    return v64GenInit1(v64GenUnlimRandom, rettyp, v64typedCreateInt(rndinc) );
}

// RETURNS: STR
// REGITRSY ALLOCATION:
// data[0] INT as lim for rnsint
// data[1] STR as pattern for printing
static inline v64Gen            v64GenCreatorUnlimStrRnd(const char *fmt, int rndinc) {
    v64Gen tmp;

     _v64GenFixRndinc(&rndinc);
    if (fmt != NULL)
        tmp = v64GenInit2(v64GenUnlimRandom,
                        VALUE64_STR,
                                v64typedCreateInt(rndinc),
                                v64typedCreateStr(fmt));
    else
        tmp = v64GenInit2(v64GenUnlimRandom, 
                        VALUE64_STR, 
                                v64typedCreateInt(rndinc),
                                V64TYPEDZERO());
    return tmp;
}



// RETURNS: FS
// REGITRSY ALLOCATION:
// data[0] INT as lim for rnsint
// data[1] STR as pattern for printing
static inline v64Gen            v64GenCreatorUnlimFsRnd(const char *fmt, int rndinc) {
    v64Gen tmp;

    _v64GenFixRndinc(&rndinc);
    if (fmt != NULL)
        tmp = v64GenInit2(v64GenUnlimRandom,
                        VALUE64_FS,
                                v64typedCreateInt(rndinc),
                                v64typedCreateStr(fmt));
    else
        tmp = v64GenInit2(v64GenUnlimRandom, 
                        VALUE64_FS, 
                                v64typedCreateInt(rndinc),
                                V64TYPEDZERO());
    return tmp;
}

// --------------------------- Creator from Source (c-str, FILE *, Ds *) series ------------

// RETURNS: VALUE64/CHR
// REGITRSY ALLOCATION:
// data[0] STR as SOURCE (no ownership)
// data[1] LONG as lim, if 0 - unlim (LONG_MAX actually)
extern v64Gen                   v64GenCreatorSourceCstrChar(const char *src, long maxlen);

// RETURNS: VALUE64/CHR
// REGITRSY ALLOCATION:
// data[0] FS as SOURCE (no ownership)
// data[1] ULONG as position
extern v64Gen                   v64GenCreatorSourceFsToChar(const fs *src);


// RETURNS: VALUE64/FS
// REGITRSY ALLOCATION:
// data[0] FS as SOURCE (no ownership)
// data[1] ULONG as position
extern v64Gen                   v64GenCreatorSourceFsToFsByNewline(const fs *src);

// RETURNS: VALUE64/STR
// REGITRSY ALLOCATION:
// data[0] FS as SOURCE (no ownership)
// data[1] ULONG as position
extern v64Gen                   v64GenCreatorSourceFsToStrByNewline(const fs *src);
// RETURNS: VALUE64/CHR
// REGITRSY ALLOCATION:
// data[0] FILE as SOURCE (no ownership)
// data[1] ULONG as remaining chars
extern v64Gen                   v64GenCreatorSourceFileChar(FILE *file);
// RETURNS: VALUE64/FS
// REGITRSY ALLOCATION:
// data[0] FILE * (no ownership)
// data[1] ULONG as remaining
// data[2] FS as buf (owner)
extern v64Gen                   v64GenCreatorSourceFileToFsByNewline(FILE *src);
// RETURNS: VALUE64/STR
// REGITRSY ALLOCATION:
// data[0] FILE * (no ownership)
// data[1] ULONG as remaining
// data[2] FS as buf (owner)
extern v64Gen                   v64GenCreatorSourceFileToStrByNewline(FILE *src);

// --------------------------------- Indirect API --------------------------------------

// ------- Stream updater -------
static inline void              v64GenStreamUpdate(v64Gen *gen, /*const char *restrict newbuf, */ long next_amount) {
    invraisecode(gen != NULL, ERR_NULLABLE_PTR, "NUll gen");

    if (gen->updater)
        gen->updater(gen, /* newbuf, */ next_amount);
    else
        userraise(0, ERR_UNSUPPORTED_GENERATOR, "Only sereval SOURCE generators support StringAppend");
}

// ------- Stream checker -------
static inline unsigned long     v64GenGetRemaining(v64Gen *gen) {
    invraisecode(gen != NULL, ERR_NULLABLE_PTR, "NUll gen");

    if (gen->remaining)
        return gen->remaining(gen);
    else
        return userraise(-1L, ERR_UNSUPPORTED_GENERATOR, "Only sereval SOURCE generators support GetRemaining");
}

static inline value64           v64GenFinalize(v64Gen *gen) {
    invraisecode(gen != NULL, ERR_NULLABLE_PTR, "NUll gen");

    if (gen->finalizer)
        return gen->finalizer(gen);
    else
        return userraise(LITERAL64_ZERO, ERR_UNSUPPORTED_GENERATOR, "Only sereval SOURCE generators support GenFinalize");
}

// ------------------------ PRINTERS/CHECKERS ---------------------------------------

extern int                      v64Techfprint(FILE *restrict out, const v64Gen *restrict gen, const char *restrict name);
static inline int               v64Techprint(const v64Gen *restrict gen, const char *restrict name) {
    return v64Techfprint(stdout, gen, name);
}

#define                         V64TECHFPRINT(out, gen) v64Techfprint( (out), &(gen), #gen)
#define                         V64TECHPRINT(gen) v64Techprint( &(gen), #gen)

// ------------------------------------ ETC. ----------------------------------------

extern bool                      v64GenValidate(FILE *restrict out, const v64Gen *restrict gen);

#endif /* !_VALUE64GEN_H */
