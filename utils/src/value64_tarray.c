/********************************************************************
             TYPED BASE VALUE64 ARRAY MODULE IMPLEMENTATION
********************************************************************/

#include "value64_tarray.h"

// ------------------------------------ Utilities ------------------------------------------


// ------------------------------------- API -----------------------------------------------


// ---------------------------------------- Testing ------------------------------------------
#ifdef VALUE64_TARRAY_TESTING

#include "test.h"

// ------------------------- TEST hset_filtereduce_* (all types) -------------------------
static TestStatus
tf(const char *name)    // TODO:
{
    logenter("%s", name);
    int subnum = 0;

    {

    }
    fs_alloc_check(true);

    return logret(TEST_PASSED, "done");
}

// ------------------------------------------------------------------------------------------------------------------------------
int
main(int argc, const char *argv[])
{
    logsimpleinit("Start");
    bool    runall = argc == 1;

    while (runall || *++argv){
        int     num = INT_MAX;    // INT_MAX for all test
        if (!runall){
            num = atoi(*argv);
            if (num < 0){
                fprintf(stderr,"Invalid test num %d\n", num);
                continue;
            }
        }
        testenginestd_run(num,
            testnew(.f2 =  tf,                                 .num = 1, .name = "Hset_in simple test"                        
                , .desc="", .mandatory=true)
        );
        if (runall)
            break;
    }

    return logret(0, "end...");  // as replace of logclose()
}

#endif /* VALUE64_TARRAY_TESTING */


