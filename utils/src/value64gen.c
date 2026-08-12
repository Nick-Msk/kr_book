/********************************************************************
                    VALUE64GEN MODULE IMPLEMENTATION
********************************************************************/

#include "value64gen.h"

// --------------------------------- CONSTANTS AND GLOBALS --------------------------

// ------------------------- CONSTRUCTOTS/DESTRUCTORS -------------------------------

value64Gen                      value64GenInit(value64GenFunc func, value64_type type, 
                                            int initcnt, value64 initdata1, value64 initdata2) {
    invraisecode(func != NULL, ERR_NULLABLE_PTR, "Generation function can't be null");
    invraisecode(value64_checktype(type), ERR_UNSUPPORTED_TYPE, "vale64 type %d isn't suppoted", type);

    value64Gen res = (value64Gen) {.fnext = func, .type = type, .counter = initcnt, 
                                .data[0] = initdata1, .data[1] = initdata2 };
    return res;
}        

// -------------------- ACCESS AND MODIFICATORS -------------------------------------

/**
 * @brief Returns the next generated value and advances the generator.
 *
 * Calls gen->next(gen) which produces the value and increments gen->counter
 * as needed (for series generators).  The caller takes ownership of the
 * returned value.
 *
 * @param gen  pointer to the generator (must not be NULL)
 * @return     the next value64 of the generator's type
 */
value64                         value64GenNext(value64Gen *gen)
{
    /* The next-function is responsible for incrementing counter when required. */
    return gen->fnext(gen);
}


// ------------------------ pre-created func V64 typed ------------------------------

/**
 * @brief Standard generator for zero / empty values.
 *
 * Returns a "zero" value of the generator's type:
 * - numeric types → 0 / 0L / 0.0 / 0UL
 * - bool          → false
 * - char          → '\0'
 * - pointer       → NULL
 * - fs            → empty fs (via value64_createfs_asstr(""))
 * - str           → empty string (via value64_createstr(""))
 *
 * The counter is left unchanged.
 *
 * @param gen  pointer to the generator (type must be set)
 * @return     a zero value64 of the requested type (ownership passed to caller)
 */
value64 value64GenZero(value64Gen *gen)
{
    switch (gen->type) {
        case VALUE64_INT:
            return value64_createint(0);
        case VALUE64_LONG:
            return value64_createlong(0L);
        case VALUE64_DBL:
            return value64_createdbl(0.0);
        case VALUE64_ULONG:
            return value64_createulong(0UL);
        case VALUE64_BOOL:
            return value64_createbool(false);
        case VALUE64_CHR:
            return value64_createchar('\0');
        case VALUE64_PTR:
            return value64_createptr(NULL);
        case VALUE64_FS:
            return value64_createfs_asstr("");
        case VALUE64_STR:
            return value64_createstr("");
        default:
            /* unknown type – return literal zero */
            return LITERAL64_ZERO;
    }
}

extern value64                  value64GenAsсSeries(value64Gen *gen);
extern value64                  value64GenAscRnd(value64Gen *gen);
extern value64                  value64GenDescSeries(value64Gen *gen);
extern value64                  value64GenDescRnd(value64Gen *gen);
extern value64                  value64GenRandom(value64Gen *gen);

// ------------------------------------ ETC. ----------------------------------------

// validation always use DIRECT fprintf (no userraise) to be able to work
// if you want to log using logging system, exec value64GenValidate(logfile, gen);
bool                      value64GenValidate(FILE *restrict out, const value64Gen *restrict gen) {
    if (!gen) {
        if (out)
            fprintf(out, "Gen is null");
        return logsimpleret(false, "Gen is null\n");
    }
    if (!gen->fnext) {
        if (out)
            fprintf(out, "Next Function is null\n");
        return logsimpleret(false, "Next Function is null");
    }
    if (!value64_checktype(gen->type)) {
        if (out)
            fprintf(out, "Value64 type %d is incorrect\n", gen->type);
        return logsimpleret(false, "Value64 type %d is incorrect", gen->type);
    }
    for (int i = 0; i < VALUE64GENCOUNT; i++) {
        if (!value64_validate(out, gen->data[i], gen->type) ) {
            if (out)
                fprintf(out, "V64 data[%d] is incorrect\n", i);
            return logsimpleret(false, "V64 data[%d] is incorrect", i);
        }
    }
    return true;
}

// ---------------------------------------- Testing ------------------------------------------
#ifdef VALUE64GEN_TESTING

#include "test.h"

//types for testing

// ------------------------- TEST init_free ---------------------------------

static TestStatus
tf_init_free(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    /* 1. int */
    test_sub("subtest %d: value64 int", ++subnum);
    {
    }
    return logret(TEST_PASSED, "done");
}

// ------------------------------------------------------------------------------------------------------------------------------
int
main(/* int argc, const char *argv[] */)
{
    logsimpleinit("Start");

    testenginestd(
        TESTADD(tf_init_free,           "Simple init and validate test")
    );

    return logret(0, "end...");  // as replace of logclose()
}

#endif /* VALUE64GEN_TESTING */

