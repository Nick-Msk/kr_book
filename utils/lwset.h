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
#include "log.h"
#include "common.h"

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
            invraisecode(ERR_OUT_OF_RANGE, values[i] >= 0 && values[i] < s.high
                , "value %hd is out of bounds for lwset", values[i]);
        if (values[i] > max)
            max = values[i];
        s.value |= (1UL << values[i]);
    }
    s.high = max;
    return s;
}

// simplifier constructor for lwset_list
#define LWSET_LIST(...) lwset_list((unsigned short[]){__VA_ARGS__}, \
    COUNT(unsigned short[]){__VA_ARGS__})


// -------------------- ACCESS AND MODIFICATORS -------------------------------------

static inline bool           lwset_get(const lwset *s, unsigned short index) {
    invraisecode(ERR_NULLABLE_PTR, s != NULL, "input pointer is NULL");
    invraisecode(ERR_OUT_OF_RANGE, index >= s->low && index <= s->high, "index %u is out of bounds for lwset", index);
    return (s->value >> index) & 1;
}

static inline bool           lwset_equals(const lwset *restrict s1, const lwset *restrict s2) {
    invraisecode(ERR_NULLABLE_PTR, s1 != NULL && s2 != NULL, 
        "Pointers is NULL %p %p", (void*) s1, (void*) s2);
    // compare even if the ranges are NOT  the same
    return s1->value == s2->value; // && s1->low == s2->low && s1->high == s2->high;
}
static inline bool           lwset_notequal(const lwset *restrict s1, const lwset *restrict s2) {
    invraisecode(ERR_NULLABLE_PTR, s1 != NULL && s2 != NULL, 
        "Pointers is NULL %p %p", (void*) s1, (void*) s2);
    return (s1->value != s2->value);  // without checking low/high, because we want to compare the actual values
}
extern bool                  lwset_in(const lwset *restrict s1, const lwset *restrict s2);

static inline bool           lwset_strictin(const lwset *restrict s1, const lwset *restrict s2) {
    return lwset_in(s1, s2) && !lwset_equals(s1, s2);
}

static inline bool           lwset_notempty(const lwset *s) {
    invraisecode(ERR_NULLABLE_PTR, s != NULL, "Pointer is NULL");
    return s->value != 0;
}

static inline bool           lwset_isempty(const lwset *s) {
    invraisecode(ERR_NULLABLE_PTR, s != NULL, "Pointer is NULL");
    return s->value == 0;
}

// Modification functions
static inline lwset          *lwset_set(lwset *s, unsigned short index, bool value) {
    invraisecode(ERR_NULLABLE_PTR, s != NULL, "input pointer is NULL");
    invraisecode(ERR_OUT_OF_RANGE, index >= s->low && index <= s->high, 
            "index %u is out of bounds for lwset", index);
    if (value)
        s->value |= (1UL << index);
    else
        s->value &= ~(1UL << index);
    return s;
}
// 
static  inline lwset           *lwset_setrange(lwset *s, unsigned short low, unsigned short high, bool value) {
    invraisecode(ERR_NULLABLE_PTR, s != NULL, "Pointer is NULL");
    invraisecode(ERR_OUT_OF_RANGE, low >= s->low && high <= s->high && low <= high, 
        "range [%u, %u] is out of bounds for lwset with range [%u, %u]", low, high, s->low, s->high);
    for (unsigned short i = low; i <= high; ++i)
        lwset_set(s, i, value);
    return s;
}

extern lwset                    *lwset_union(lwset *restrict s1, const lwset *restrict s2);
extern lwset                    *lwset_intersect(lwset *restrict s1, const lwset *restrict s2);
extern lwset                    *lwset_minus(lwset *restrict s1, const lwset *restrict s2);
extern lwset                    *lwset_simmdiff(lwset *restrict s1, const lwset *restrict s2);

// extern lwset                    *lwset_include(lwset *restrict s1, const lwset *restrict s2);
// extern lwset                    *lwset_exclude(lwset *restrict s1, const lwset *restrict s2);


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