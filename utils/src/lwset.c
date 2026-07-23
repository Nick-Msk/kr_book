/********************************************************************
               LIGHTWEIGHT HASH SET MODULE IMPLEMENTATION
********************************************************************/

#include "lwset.h"

// ------------------------------------ Utilities ------------------------------------------

// ------------------------------------- API -----------------------------------------------

// ------------------------ PRINTERS/CHECKERS ---------------------------------------

int                      lwset_techfprint(FILE *restrict out, const lwset *restrict s) {
    int cnt = 0;
    if (out) {
    invraisecode(ERR_NULLABLE_PTR, s != NULL, 
        "Pointers is NULL %p", (void*) s);
    
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

// ------------------------- TEST lwset init ---------------------------------
static TestStatus
tf_lwset_init(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    /* ========== Инициализация ========== */
    test_sub("subtest %d: lwset_initunlim creates empty set [0,63]", ++subnum);
    {
        lwset s = lwset_initunlim();
        test_validate(
            s.value == 0 && s.low == 0 && s.high == 63,
            "initunlim: value=0x%016llx, low=%u, high=%u (expected 0, 0, 63)",
            (unsigned long long)s.value, s.low, s.high
        );
    }

    test_sub("subtest %d: lwset_init1unlim creates full set [0,63]", ++subnum);
    {
        lwset s = lwset_init1unlim();
        test_validate(
            s.value == UINT64_MAX && s.low == 0 && s.high == 63,
            "init1unlim: value=0x%016llx (expected all ones)", (unsigned long long)s.value
        );
    }

    test_sub("subtest %d: lwset_init0 with range [10,20]", ++subnum);
    {
        lwset s = lwset_init0(10, 20);
        test_validate(
            s.value == 0 && s.low == 10 && s.high == 20,
            "init0 range: value=0x%llx, low=%u, high=%u", (unsigned long long)s.value, s.low, s.high
        );
    }

    test_sub("subtest %d: lwset_init1 with range [5,10]", ++subnum);
    {
        lwset s = lwset_init1(5, 10);
        uint64_t expected = (1ULL << (10 - 5 + 1)) - 1;  // 6 бит = 0x3F
        test_validate(
            s.value == expected && s.low == 5 && s.high == 10,
            "init1 range: value=0x%llx (expected 0x%llx), low=%u, high=%u",
            (unsigned long long)s.value, (unsigned long long)expected, s.low, s.high
        );
    }

    /* ========== Проверка подмножества (lwset_in) ========== */
    test_sub("subtest %d: empty set is subset of any set", ++subnum);
    {
        lwset empty = lwset_initunlim();
        lwset full  = lwset_init1unlim();
        test_validate(
            lwset_in(&empty, &full),
            "Empty set must be subset of full set"
        );
        test_validate(
            lwset_in(&empty, &empty),
            "Empty set must be subset of itself"
        );
    }

    test_sub("subtest %d: full set is subset of itself only", ++subnum);
    {
        lwset full = lwset_init1unlim();
        lwset empty = lwset_initunlim();
        test_validate(
            lwset_in(&full, &full),
            "Full set is subset of itself"
        );
        test_validate(
            !lwset_in(&full, &empty),
            "Full set is NOT subset of empty set"
        );
    }

    test_sub("subtest %d: subset with single bit", ++subnum);
    {
        lwset s1 = lwset_initunlim();
        s1.value = (1ULL << 5);          // только бит 5
        lwset s2 = lwset_initunlim();
        s2.value = (1ULL << 5) | (1ULL << 10);
        test_validate(
            lwset_in(&s1, &s2),
            "Single bit set must be subset of larger set"
        );
        test_validate(
            !lwset_in(&s2, &s1),
            "Larger set is NOT subset of smaller set"
        );
    }

    test_sub("subtest %d: subsets with disjoint ranges", ++subnum);
    {
        lwset s1 = lwset_init0(0, 10);
        s1.value = (1ULL << 2) | (1ULL << 5);  // биты 2 и 5 в диапазоне 0–10

        lwset s2 = lwset_init0(20, 30);
        s2.value = (1ULL << 25) | (1ULL << 27); // биты 25 и 27 в диапазоне 20–30

        test_validate(
            !lwset_in(&s1, &s2),
            "Sets with different ranges and non-overlapping bits must not be subsets"
        );

        test_validate(
            !lwset_in(&s2, &s1),
            "Reverse check must also fail"
        );
    }

    /* ========== Обработка ошибок ========== */
    test_sub("subtest %d: NULL pointer raises SIGINT", ++subnum);
    {
        lwset s = lwset_initunlim();
        if (!try()) {
            lwset_in(NULL, &s);
            test_validate(false, "Should have raised SIGINT for NULL s1");
        } else {
            logsimple("Exception correctly raised on NULL s1");
        }
        if (!try()) {
            lwset_in(&s, NULL);
            test_validate(false, "Should have raised SIGINT for NULL s2");
        } else {
            logsimple("Exception correctly raised on NULL s2");
        }
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
                fprintf(stderr, "Invalid test num %d\n", num);
                continue;
            }
        }
            testenginestd_run(num,
                testnew(.f2 =  tf_lwset_init,  .num =  1, .name = "Lwset init simple test"                        , 
                        .desc="", .mandatory=true)
                 );
        if (runall)
            break;
    }

    return logret(0, "end...");  // as replace of logclose()
}

#endif /* LWSET_TESTING */
