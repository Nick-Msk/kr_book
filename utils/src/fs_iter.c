#include "common.h"
#include "log.h"
#include "fs.h"
#include "error.h"

/********************************************************************
                    FAST STRING MODULE IMPLEMENTATION
********************************************************************/

// ---------- pseudo-header for utility procedures -----------------

// -------------------------- (Utility) printers -------------------


// ------------------------------ Utilities -------------- ----------

// --------------------------- API ---------------------------------

// ------------------ General functions ----------------------------

// -------------------------- (API) printers -----------------------

// ------------------ API Constructs/Destrucor  ----------------------------

// -------------------------------Testing --------------------------

#ifdef FSITERTESTING

#include "test.h"
#include "checker.h"

//types for testing

// ------------------------- TEST 1 ---------------------------------

static TestStatus
tf1(const char *name)
{
    logenter("%s", name);
    int         subnum = 0;
    {
        test_sub("subtest %d", ++subnum);

    return logret(TEST_PASSED, "done"); // TEST_FAILED
}

// ------------------------------------------------------------------

int
main(/* int argc, char *argv[] */)
{
    logsimpleinit("Starting");   // it that working?

    testenginestd(
        testnew(.f2 = tf1, .num = 1, .name = "Simple init and validate test" , .desc="Init test."                , .mandatory=true)
    );

    logclose("end...");
    return 0;
}

#endif /* FSITERTESTING */

