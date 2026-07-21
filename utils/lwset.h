#ifndef _LWSET_H
#define _LWSET_H

// ---------------------------------------------------------------------------------
// -------------------------- Public lightweight API -------------------------------
// ---------------------------------------------------------------------------------

// ----------------------------- Includes ------------------------------------------

#include <inttypes.h>
#include <stdio.h>

// --------------------------------- CONSTANTS AND GLOBALS --------------------------

// ---------------------------------- TYPES -----------------------------------------

typedef struct {
    int64_t value;  // now only 1 int64_t value is used, but in the future, we can add more fields if needed
} lwset;

// ------------------------- CONSTRUCTOTS/DESTRUCTORS -------------------------------

static inline lwset lwset_init(void) {
    return (lwset) {.value = 0};
}

// -------------------- ACCESS AND MODIFICATORS -------------------------------------



// ----------------------------- CONVERTERS ----------------------------------------

// TODO:
// hset *lwset_to_hset(const lwset *s); create  a new VALUE64_INT hset

// ------------------------ PRINTERS/CHECKERS ---------------------------------------

// --------------------------------- SERIALIZATION ----------------------------------

// ------------------------------------ ETC. ----------------------------------------

#endif /* !_LWSET_H */