/********************************************************************
               LIGHTWEIGHT HASH SET MODULE IMPLEMENTATION
********************************************************************/

#include "lwset.h"

// ------------------------------------ Utilities ------------------------------------------

// ------------------------------------- API -----------------------------------------------

// ------------------------ PRINTERS/CHECKERS ---------------------------------------
/// @brief Technical Prints the lwset to the specified output stream
/// @param s pointer to the lwset
/// @param out output stream (e.g., stdout, stderr, or a file), CAN be NULL
/// @return number of characters printed    
int                      lwset_techfprint(FILE *restrict out, const lwset *restrict s) {
    int cnt = 0;
    if (out) {
    invraisecode(ERR_NULLABLE_PTR, s != NULL, 
        "Pointers is NULL %p %p", (void*) s);
    
        cnt +=  fprintf(out, "LWSET {value=");
        // print each bit in the range [low, high] as 0 or 1
        uint64_t tmpval = s->value;
        // logging in logfile with oiffset! For each line, print the offset spaces before the actual content
        if (out == logfile)
            logprintoffset();
        for (unsigned short i = s->low; i <= s->high; tmpval >>= 1) {
            if (tmpval == 0)
                cnt += fprintf(out, "|");   // no more bits set, print a separator
            cnt += fprintf(out, "%c", tmpval & 1 ? '1' : '0');
        }
        cnt += fprintf(out, ", low=%u, high=%u }\n", s->low, s->high);
    }
    return cnt;
}

// --------------------------------------- ITERATORS ---------------------------------------

// --------------------------------- SERIALIZATION -----------------------------------------

// TODO:

// ---------------------------------------- Testing ------------------------------------------
#ifdef LWSET_TESTING

#include "test.h"

// ------------------------- TEST 8 ---------------------------------
static TestStatus
tf8(const char *name)
{
    logenter("%s", name);
    int subnum = 0;
    test_sub("subtest %d: FS vs INT type mismatch raise SIGINT", ++subnum);
    {
    }

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
                testnew(.f2 =  tf8,  .num =  1 .name = "Lwset simple test"                        , 
                        .desc="", .mandatory=true)

                 );
        if (runall)
            break;
    }

    return logret(0, "end...");  // as replace of logclose()
}

#endif /* LWSET_TESTING */
