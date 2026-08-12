/**
 * @file value64gen.h
 * @brief Generator!
 */

#ifndef _VALUE64GEN_H
#define _VALUE64GEN_H

// ---------------------------------------------------------------------------------
// ------------------------------ Public value64 API -------------------------------
// ---------------------------------------------------------------------------------

// ----------------------------- Includes ------------------------------------------

#include <stdio.h>
#include "value64.h"
#include "error.h"

// --------------------------------- CONSTANTS AND GLOBALS --------------------------

enum value64GenConstants{
                                        VALUE64GENCOUNT = 2
};

// ---------------------------------- TYPES -----------------------------------------

typedef struct value64Gen               value64Gen;
typedef value64                         (*value64GenFunc)(value64Gen *gen);

typedef struct value64Gen {
    value64GenFunc  fnext;
    value64_type    type;
    unsigned int    counter;
    value64         data[VALUE64GENCOUNT];
} value64Gen;

// ------------------------- CONSTRUCTOTS/DESTRUCTORS -------------------------------

extern value64Gen               value64GenInit(value64GenFunc func, value64_type type, 
                                               value64 initdata1, value64 initdata2);
static inline value64Gen        value64GenInit0(value64GenFunc func, value64_type type) {
    return value64GenInit(func, type, LITERAL64_ZERO, LITERAL64_ZERO);
}  
static inline value64Gen        value64GenInit1(value64GenFunc func, value64_type type, value64 initdata1) {
    return value64GenInit(func, type, initdata1, LITERAL64_ZERO);
}    
                     
static inline void              value64GenFree(value64Gen *gen) {
    if (gen) {
        for (int i = 0; i < VALUE64GENCOUNT; i++)
            value64_free(&gen->data[i], gen->type);    // even if data = 0LL
    }
}

#define VALUE64GEN_ZERO (value64Gen) {.next = NULL, .type = VALUE64_UNKNOWN, .counter = 0, \
        .data = {LITERAL64_ZERO, LITERAL64_ZERO} }
#define VALUE64GENFREE(gen) { value64GenFree(gen); gen = VALUE64GEN_ZERO; }

// -------------------- ACCESS AND MODIFICATORS -------------------------------------

// ------------- GENERALLIZED ACCESS -----------------
extern value64                  value64GenNext(value64Gen *gen);
extern value64                  value64GenCurr(value64Gen *gen);

// ------------------ pre-created func V64 typed -----------------------------

extern value64                  value64GenUnlimZero(value64Gen *gen);
// unchecker group. 0 -> 1 -> 2 ... INT_MAX -> INT_MIN etc...
// for bool false -> true -> false ...
// EXCEPT double. That type w/o cycling.
extern value64                  value64UnchecGenUnlimAsсSeries(value64Gen *gen);
extern value64                  value64UnchecGenUnlimAscRnd(value64Gen *gen);
extern value64                  value64UnchecGenUnlimDescSeries(value64Gen *gen);
extern value64                  value64UnchecGenUnlimDescRnd(value64Gen *gen);
extern value64                  value64UnchecGenUnlimRandom(value64Gen *gen);
// check group: TODO:

// ------------------------ PRINTERS/CHECKERS ---------------------------------------

extern int                      value64Techfprint(FILE *restrict out, const value64Gen *restrict gen);
static inline int               value64Techprint(const value64Gen* gen) {
    return value64Techfprint(stdout, gen);
}

// ------------------------------------ ETC. ----------------------------------------

extern bool                      value64GenValidate(FILE *restrict out, const value64Gen *restrict gen);  // TODO:

#endif /* !_VALUE64GEN_H */
