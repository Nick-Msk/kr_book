#ifndef _LWSET_H
#define _LWSET_H

// ---------------------------------------------------------------------------------
// -------------------------- Public lightweight API -------------------------------
// ---------------------------------------------------------------------------------

// ----------------------------- Includes ------------------------------------------

#include <inttypes.h>
#include <stdio.h>

#include "checker.h"
#include "error.h"

// --------------------------------- CONSTANTS AND GLOBALS --------------------------

// ---------------------------------- TYPES -----------------------------------------

typedef struct {
    uint64_t value;  // now only 1 int64_t value is used, but in the future, we can add more fields if needed
    unsigned short low, high;  
} lwset;

// ------------------------- CONSTRUCTOTS/DESTRUCTORS -------------------------------

static inline lwset             lwset_initunlim(void) {
    return (lwset) {.value = 0, .low = 0, .high = sizeof(int64_t) * 8 - 1};  // set high to the maximum bit index for int64_t
}

static inline lwset             lwset_init0(unsigned short low, unsigned short high) {
    return (lwset) {.value = 0, .low = low, .high = high};
}

static inline lwset             lwset_clone(const lwset *s) {
    invraisecode(ERR_NULLABLE_PTR, s != NULL, "input pointer is NULL");
    return (lwset) {.value = s->value, .low = s->low, .high = s->high};
}

static inline lwset             lwset_list(const unsigned short *values, size_t count) {
    lwset s = lwset_initunlim();
    unsigned short max = 0;
    for (size_t i = 0; i < count; ++i) {
            invraisecode(ERR_OUT_OF_BOUNDS, values[i] >= 0 && values[i] < s.high
                , "value %hd is out of bounds for lwset", values[i]);
        if (values[i] > max)
            max = values[i];
        s.value |= (1UL << values[i]);
    }
    s.high = max;
    return s;
}


// -------------------- ACCESS AND MODIFICATORS -------------------------------------

static inline bool           lwset_get(const lwset *s, unsigned short index) {
    invraisecode(ERR_NULLABLE_PTR, s != NULL, "input pointer is NULL");
    invraisecode(ERR_OUT_OF_RANGE, index >= s->low && index <= s->high, "index %u is out of bounds for lwset", index);
    return (s->value >> index) & 1;
}

static inline void           lwset_set(lwset *s, unsigned short index, bool value) {
    invraisecode(ERR_NULLABLE_PTR, s != NULL, "input pointer is NULL");
    invraisecode(ERR_OUT_OF_RANGE, index >= s->low && index <= s->high, "index %u is out of bounds for lwset", index);
    if (value)
        s->value |= (1UL << index);
    else
        s->value &= ~(1UL << index);
}

extern lwset                *lwset_union(const lwset *restrict s1, const lwset *restrict s2);
extern lwset                *lwset_intersect(const lwset *restrict s1, const lwset *restrict s2);
extern lwset                *lwset_minus(const lwset *restrict s1, const lwset *restrict s2);
static inline bool           lwset_equals(const lwset *restrict s1, const lwset *restrict s2) {
    invraisecode(ERR_NULLABLE_PTR, s1 != NULL && s2 != NULL, "input pointer is NULL");
    // compare even if the ranges are NOT  the same
    return s1->value == s2->value; // && s1->low == s2->low && s1->high == s2->high;
}

// ----------------------------- CONVERTERS ----------------------------------------

// TODO:
// hset *lwset_to_hset(const lwset *s); create  a new VALUE64_INT hset

// ------------------------ PRINTERS/CHECKERS ---------------------------------------

extern int                      lwset_fprint(const lwset *restrict s, FILE *restrict out);
static inline int               lwset_print(const lwset * s) {
    return lwset_fprint(s, stdout);
}

// --------------------------------- SERIALIZATION ----------------------------------

// ------------------------------------ ETC. ----------------------------------------

#endif /* !_LWSET_H */