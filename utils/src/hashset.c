/********************************************************************
                    HASH-BASED SET MODULE IMPLEMENTATION
********************************************************************/

// ---------- pseudo-header for utility procedures -----------------

// ---------------------------- (Utility) printers -----------------------------


// ------------------------------------ Utilities ------------------------------------------

// ------------------------------------- API -----------------------------------------------

// ---------------------------------- API Constructs/Destrucor  ----------------------------

// ---------------------------------- General functions ------------------------------------

// ------------------------------------- (API) printers ------------------------------------

// --------------------------------- SERIALIZATION -----------------------------------------

// -------------------------------Testing --------------------------
#ifdef HSETTESTING

#include "test.h"
#include "checker.h"

//types for testing

// ------------------------- TEST 1 ---------------------------------

static TestStatus
tf1(const char *name)
{
    logenter("%s", name);
    int         subnum = 0;

    test_sub("subtest %d: ........", ++subnum);
    {
        
    }
    return logret(TEST_PASSED, "done"); // TEST_FAILED, TEST_PASSED, TEST_MANUAL
}

// ------------------------------------------------------------------------------------------------------------------------------
int
main( /* int argc, const char *argv[] */)
{
    logsimpleinit("Start");

    testenginestd(
        testnew(.f2 = tf1,  .num =  1, .name = "Simple init and validate test"              , .desc="", .mandatory=true)
    );

    return logret(0, "end...");  // as replace of logclose()
}

#endif /* HSETTESTING */

