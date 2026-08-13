/********************************************************************
                    VALUE64 TYPED MODULE IMPLEMENTATION
********************************************************************/

#include "v64typed.h"

// ---------------------------------- CONSTANTS AND GLOBALS --------------------------

// ---------------------------- CONSTRUCTORS / DESTRUCTORS ---------------------------

// ------------------------ PRINTERS/CHECKERS ---------------------------------------

int                      v64typedTechfprint(FILE *out, v64typed tval) {
    int cnt = 0;
    if (out) {
        // TODO:
    }
    return cnt;
}

// ------------------------------------ ETC. ----------------------------------------

bool                     v64typedValidate(FILE *out, v64typed tval) {

}

// ---------------------------------------- Testing ------------------------------------------
#ifdef V64TYPED_TESTING

#include "test.h"

//types for testing

static TestStatus
tf1_init_free(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    /* ========== INT ========== */
    test_sub("subtest %d: ...", ++subnum);
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
        TESTADD(tf1_init_free,           "Simple init and validate test")
    );

    return logret(0, "end...");  // as replace of logclose()
}

#endif /* V64TYPED_TESTING */


