#include "common.h"
#include "log.h"
#include "fs.h"
#include "error.h"
#include "fs_iter.h"

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

        const char *arr = "abcdefgh";
        fs s = fsliteral(arr);
        for (fsiter i = fseach(s); hasnext(i); next(i) )
            if (curr(i) != *arr++)
                return logerr(TEST_FAILED, "current iterator value [%c] != [%c]", curr(i), *(arr - 1));

        test_sub("subtest %d", ++subnum);
        fs      str = fsinit(100);
        int     pos = 0;
        for (fsiter i = fseach(s); hasnext(i); next(i) )
            elem(str, pos++) = curr(i);
        fsend(str, pos);
        if (fscmp(str, s) != 0)
            return logactret(fsfree(str), TEST_FAILED, "[%s] not equal to origin [%s]", fsstr(str), fsstr(s) );

        test_sub("subtest %d", ++subnum);

        pos = 0;
        for (fsiter i = fsslice(s, 4, 11); hasnext(i); next(i) )
            elem(str, pos++) = curr(i);
        fsend(str, pos);
        if (strcmp(s.v + 4, fsstr(str) ) != 0)
            return logactret(fsfree(str), TEST_FAILED, "[%s] not equal to origin [%s]", fsstr(str), s.v + 4);
        fsfree(str);
    }
    return logret(TEST_PASSED, "done"); // TEST_FAILED
}

// ------------------------- TEST 2 ---------------------------------

static TestStatus
tf2(const char *name)
{
    logenter("%s", name);
    int         subnum = 0;
    {
        test_sub("subtest %d", ++subnum);

        fs      str = fsinit(100);
        const char *arr = "abcdefgh";
        fs      s = fsliteral(arr);
        int     pos = 0;
        for (fsiterrev i = fseachrev(s); hasnextrev(i); nextrev(i) )
            elem(str, pos++) = currrev(i);
        fsend(str, pos);

        printf("BEFORE REV: %s\n", fsstr(str) );
        fs_reverse(str);

        if (fscmp(str, s) != 0)
            return logactret(fsfree(str), TEST_FAILED, " REVERSED [%s] not equal to origin [%s]", fsstr(str), fsstr(s) );
        fsfree(str);
    }
    return logret(TEST_PASSED, "done"); // TEST_FAILED
}

// ------------------------------------------------------------------

int
main(int argc, const char *argv[])
{
    const char *logfilename = "log/fs_iter.log";

    loginit(logfilename, false, 0, "Starting");

    if (argc > 1)
        logfilename = argv[1];

    testenginestd(
        testnew(.f2 = tf1, .num = 1, .name = "Simple iteration test" ,          .desc="", .mandatory=true)
      , testnew(.f2 = tf2, .num = 2, .name = "Reverse iteration test" ,         .desc="", .mandatory=true)
    );

    logclose("end...");
    return 0;

}

#endif /* FSITERTESTING */

