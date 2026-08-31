#ifndef _STACK_H
#define _STACK_H

#include <stdio.h>
#include "bool.h"

// ---------------------------------------------------------------------------------
// --------------------------- Public STACK API ----------------------------------
// ---------------------------------------------------------------------------------

// ----------- CONSTANTS AND GLOBALS ---------------

// ------------------- TYPES -----------------------

// ------------- CONSTRUCTOTS/DESTRUCTORS ----------

extern void                    stack_clear(void);

// -------------- ACCESS AND MODIFICATION ----------

extern double                  stack_pop(void);

extern double                  stack_get(void);

extern bool                    stack_push(double val);

extern double                  stack_exch(void);


extern int                     stack_count(void);

static inline bool             stack_pushsame(void){
    return stack_push(stack_get());
}

// ----------------- PRINTERS ----------------------

extern int                     stack_fprint(FILE *f);

static inline int              stack_print(void){
    return stack_fprint(stdout);
}

// ------------------ ETC. -------------------------

#endif /* !_STACK_H */

