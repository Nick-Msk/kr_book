/********************************************************************
                    VALUE64GEN MODULE IMPLEMENTATION
********************************************************************/

#include "value64gen.h"

// --------------------------------- CONSTANTS AND GLOBALS --------------------------

// ------------------------- CONSTRUCTOTS/DESTRUCTORS -------------------------------

value64Gen               value64GenInit(value64GenFunc func, value64_type type, 
                                            int initcnt, value64 initdata1, value64 initdata2) {
                                    
}

// -------------------- ACCESS AND MODIFICATORS -------------------------------------

// ------------------------ pre-created func V64 typed ------------------------------

// ------------------------------------ ETC. ----------------------------------------

bool                      value64GenValidate(value64 v, value64_type typ) {

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

