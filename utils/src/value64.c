/********************************************************************
                    VALUE64(128) SET MODULE IMPLEMENTATION
********************************************************************/

// common include
#include "value64.h"

static const size_t                     value_sizes[] = {
    [VALUE64_INT]  = sizeof(int),
    [VALUE64_LNG] = sizeof(long),
    [VALUE64_DBL]  = sizeof(double),
    [VALUE64_PTR]  = sizeof(void *),
    [VALUE64_STR]  = sizeof(char *),
    [VALUE64_FS]   = sizeof(fs *)
};


// create value from pointer, value64 constructor ANY type, MOVE semantic
value64                   value64_pcopy_move(void *p, value64_type typ, bool move){
    value64     tmp = VALUE64_ZERO;  // init
    switch (typ){
        case VALUE64_INT:
            tmp.ival = *(const int *) p;
        break;
        case VALUE64_LNG:
            tmp.lval = *(const long *) p;
        break;
        case VALUE64_DBL:
            tmp.dval = *(const double *) p;
        break;
        case VALUE64_PTR:
            tmp.pval = *(void * const *) p;
        break;
        case VALUE64_STR:
            if (move)
                tmp.sval = (char *) p;  //MOVE POINTER
            else {
                if ( (tmp.sval = strdup(p) ) == NULL)
                    userraiseint(ERR_UNABLE_ALLOCATE, "Unable to dup string");
            }
        break;
        case VALUE64_FS:
            if (move)
                // create fs body in head with FS_FLAG_BODYALLOC
                tmp.fsval = fs_moveto_heap(p);
            else
                tmp.fsval = fs_heapcreate(p);
        break;
        default:
            userraiseint(ERR_UNSUPPORTED_TYPE, "type %d %s isn't suppoted", typ, value64_type_name(typ) );
        break;
    }
    return tmp;
}

// create value from pointer, value64 constructor ANY type, MOVE semantic
value64                   value64_pinit(const void *p, value64_type typ){
    value64     tmp = VALUE64_ZERO;  // init
    switch (typ){
        case VALUE64_INT:
            tmp.ival = *(const int *) p;
        break;
        case VALUE64_LNG:
            tmp.lval = *(const long *) p;
        break;
        case VALUE64_DBL:
            tmp.dval = *(const double *) p;
        break;
        case VALUE64_PTR:
            tmp.pval = *(void * const *) p;
        break;
        case VALUE64_STR:
            tmp.sval = (char *) p;  //MOVE POINTER
        break;
        case VALUE64_FS:
            // create fs body in head with FS_FLAG_BODYALLOC
            tmp.fsval = fs_heapcreate(p); //*(fs * const *) p;  // probable fs_healcreate() is required
        break;
        default:
            userraiseint(ERR_UNSUPPORTED_TYPE, "type %d %s isn't suppoted", typ, value64_type_name(typ) );
        break;
    }
    return tmp;
}

// --------------------------------- SERIALIZATION -----------------------------------------

// ---------------------------------------- Testing ------------------------------------------
#ifdef VALUE64TESTING

#include "test.h"
#include "array.h"
#include <time.h>

//types for testing

// ------------------------- TEST 1 ---------------------------------

static TestStatus
tf1(const char *name)
{
    logenter("%s", name);
    int         subnum = 0;

    test_sub("subtest %d: init + free", ++subnum);
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
    printf("%d\n", argc);

    while (runall || *++argv){
        int     num = INT_MAX;    // INT_MAX for all test
        if (!runall){
            num = atoi(*argv);
            if (num < 0){
                fprintf(stderr,"Invalid test num %d\n", num);
                continue;
            }
        }
        printf("Num %d\n", num);
            testenginestd_run(num,
                testnew(.f2 =  tf1,  .num =  1, .name = "Simple init and validate test"              , .desc="", .mandatory=true)
            );
        if (runall)
            break;
    }

    return logret(0, "end...");  // as replace of logclose()
}

#endif /* HSETTESTING */


