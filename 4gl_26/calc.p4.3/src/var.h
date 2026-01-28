#ifndef _VAR_H
#define _VAR_H

#include <stdio.h>
#include "bool.h"

// ---------------------------------------------------------------------------------
// --------------------------- Public STACK API ----------------------------------
// ---------------------------------------------------------------------------------

// ----------- CONSTANTS AND GLOBALS ---------------

// ------------------- TYPES -----------------------

// ------------- CONSTRUCTOTS/DESTRUCTORS ----------

extern void                    var_clear(void);

// -------------- ACCESS AND MODIFICATION ----------

extern double                  var_get(char c);

extern double                  var_set(char c, double val);

// ----------------- PRINTERS ----------------------

extern int                     var_fprint(FILE *f);

static inline int              var_print(void){
    return var_fprint(stdout);
}

// ------------------ ETC. -------------------------

#endif /* !_VAR_H */

