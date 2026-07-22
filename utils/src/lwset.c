/********************************************************************
               LIGHTWEIGHT HASH SET MODULE IMPLEMENTATION
********************************************************************/

#include "lwset.h"

// ------------------------------------ Utilities ------------------------------------------


// ------------------------------------- API -----------------------------------------------

/// @brief Checks if s1 is a subset of s2
/// @param s1 first lwset pointer
/// @param s2 second lwset pointer
/// @return true if s1 is a subset of s2, false otherwise
bool                     lwset_in(const lwset *restrict s1, const lwset *restrict s2) {
    invraisecode(ERR_NULLABLE_PTR, s1 != NULL && s2 != NULL, 
        "Pointers is NULL %p %p", (void*) s1, (void*) s2);
    return (s1->value & s2->value) == s1->value;
}
/// @brief  Computes the union of two lwsets and stores the result in s1  
/// @param s1   first lwset pointer
/// @param s2   second lwset pointer
/// @return  pointer to the modified s1
lwset                    *lwset_union(lwset *restrict s1, const lwset *restrict s2) {
    invraisecode(ERR_NULLABLE_PTR, s1 != NULL && s2 != NULL, 
        "Pointers is NULL %p %p", (void*) s1, (void*) s2);
    s1->value |= s2->value;
    return s1;
}
/// @brief  Computes the intersection of two lwsets and stores the result in s1
/// @param s1   first lwset pointer
/// @param s2   second lwset pointer
/// @return  pointer to the modified s1
lwset                    *lwset_intersect(lwset *restrict s1, const lwset *restrict s2) {
    invraisecode(ERR_NULLABLE_PTR, s1 != NULL && s2 != NULL, 
        "Pointers is NULL %p %p", (void*) s1, (void*) s2);
    s1->value &= s2->value;
    return s1;
}
/// @brief  Computes the difference of two lwsets and stores the result in s1
/// @param s1   first lwset pointer
/// @param s2   second lwset pointer
/// @return  pointer to the modified s1
lwset                    *lwset_minus(lwset *restrict s1, const lwset *restrict s2) {
    invraisecode(ERR_NULLABLE_PTR, s1 != NULL && s2 != NULL, 
        "Pointers is NULL %p %p", (void*) s1, (void*) s2);
    s1->value &= ~s2->value;
    return s1;
}
/// @brief Computes the symmetric difference of two lwsets and stores the result in s1
/// @param s1  first lwset pointer
/// @param s2  second lwset pointer
/// @return  pointer to the modified s1
lwset                    *lwset_simmdiff(lwset *restrict s1, const lwset *restrict s2) {
    invraisecode(ERR_NULLABLE_PTR, s1 != NULL && s2 != NULL, 
        "Pointers is NULL %p %p", (void*) s1, (void*) s2);
    s1->value ^= s2->value;
    return s1;
}



// --------------------------------------- ITERATORS ---------------------------------------

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