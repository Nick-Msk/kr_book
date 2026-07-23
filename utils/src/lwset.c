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

// ------------------------- TEST lwset_clone / lwset_list / LWSET_LIST -------------------------
static TestStatus
tf_lwset_clone_list(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    /* ========== lwset_clone ========== */
    test_sub("subtest %d: clone empty set", ++subnum);
    {
        lwset orig = lwset_initunlim();
        lwset copy = lwset_clone(&orig);
        test_validate(
            copy.value == orig.value &&
            copy.low == orig.low &&
            copy.high == orig.high,
            "Clone empty: value=0x%llx low=%u high=%u (expected same as orig)",
            (unsigned long long)copy.value, copy.low, copy.high
        );
        // Изменение оригинала не влияет на копию
        orig.value = 0xAA;
        test_validate(
            copy.value == 0,
            "Clone must be independent: orig changed, copy still 0"
        );
    }

    test_sub("subtest %d: clone non‑empty set", ++subnum);
    {
        lwset orig = lwset_initunlim();
        orig.value = (1ULL << 3) | (1ULL << 15);
        orig.high = 20;
        lwset copy = lwset_clone(&orig);
        test_validate(
            copy.value == orig.value &&
            copy.low == orig.low &&
            copy.high == orig.high,
            "Clone non‑empty: value=0x%llx low=%u high=%u (expected same as orig)",
            (unsigned long long)copy.value, copy.low, copy.high
        );
    }

    /* ========== lwset_list ========== */
    test_sub("subtest %d: list with empty array", ++subnum);
    {
        unsigned short vals[] = {};
        lwset s = lwset_list(vals, 0);
        test_validate(
            s.value == 0 && s.low == 0 && s.high == 0,   // high остаётся 0? в коде high изначально 63, но затем max=0 -> s.high = 0
            "List empty: value=0x%llx low=%u high=%u (expected 0,0,0)",
            (unsigned long long)s.value, s.low, s.high
        );
    }

    test_sub("subtest %d: list with several values", ++subnum);
    {
        unsigned short vals[] = {2, 7, 15, 63};
        lwset s = lwset_list(vals, COUNT(vals));
        // проверяем, что биты 2,7,15,63 установлены
        test_validate(
            (s.value & (1ULL << 2))  != 0 &&
            (s.value & (1ULL << 7))  != 0 &&
            (s.value & (1ULL << 15)) != 0 &&
            (s.value & (1ULL << 63)) != 0 &&
            s.high == 63,
            "List {2,7,15,63}: bits 2,7,15,63 must be set, high=%u", s.high
        );
        // лишних битов нет (только эти 4)
        test_validate(
            s.value == ((1ULL << 2) | (1ULL << 7) | (1ULL << 15) | (1ULL << 63)),
            "List value must exactly match expected bits"
        );
    }

    test_sub("subtest %d: list with duplicates", ++subnum);
    {
        unsigned short vals[] = {5, 5, 5};
        lwset s = lwset_list(vals, COUNT(vals));
        test_validate(
            s.value == (1ULL << 5) && s.high == 5,
            "Duplicates: only bit 5 must be set"
        );
    }

    test_sub("subtest %d: LWSET_LIST macro", ++subnum);
    {
        lwset s = LWSET_LIST(0, 10, 20);
        test_validate(
            (s.value & (1ULL << 0))  != 0 &&
            (s.value & (1ULL << 10)) != 0 &&
            (s.value & (1ULL << 20)) != 0 &&
            s.high == 20,
            "LWSET_LIST(0,10,20): bits 0,10,20 must be set"
        );
    }

    /* ========== Ошибка выхода за диапазон ========== */
    test_sub("subtest %d: list with value >= 64 raises SIGINT", ++subnum);
    {
        if (!try()) {
            unsigned short vals[] = {64};  // 64 > 63
            lwset s = lwset_list(vals, 1);
            // не должны сюда попасть
            test_validate(false, "Should have raised SIGINT for value 64");
            (void)s;
        } else {
            logsimple("Exception correctly raised on value 64");
        }
    }

    return logret(TEST_PASSED, "done");
}

// ------------------------- TEST lwset_get / lwset_equals / lwset_notequal  -------------------------
static TestStatus
tf_lwset_get_equals(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    /* ========== lwset_get ========== */
    test_sub("subtest %d: get bit from empty set", ++subnum);
    {
        lwset s = lwset_initunlim();
        test_validate(
            lwset_get(&s, 0) == false,
            "Empty set: bit 0 must be false"
        );
    }

    test_sub("subtest %d: get set bit", ++subnum);
    {
        lwset s = lwset_initunlim();
        s.value |= (1ULL << 5);
        test_validate(
            lwset_get(&s, 5) == true,
            "Set: bit 5 must be true"
        );
        test_validate(
            lwset_get(&s, 4) == false,
            "Set: bit 4 must be false (only bit 5 is set)"
        );
    }

    test_sub("subtest %d: get bit at boundary high=63", ++subnum);
    {
        lwset s = lwset_initunlim();
        s.value = UINT64_MAX;  // все биты установлены
        test_validate(
            lwset_get(&s, 63) == true,
            "Bit 63 must be true when full set"
        );
    }

    test_sub("subtest %d: get with index out of range raises", ++subnum);
    {
        lwset s = lwset_init0(0, 10);
        
        // 1. Валидный индекс – исключения быть не должно
        bool valid = lwset_get(&s, 0);
        test_validate(valid == false, "Bit 0 in empty set must be false");
        
        // 2. Невалидный индекс – должно выброситься исключение
        if (!try()) {
            lwset_get(&s, 11);   // index 11 > high (10)
            test_validate(false, "Should have raised SIGINT for index 11");
        } else {
            logsimple("Exception correctly raised for index 11");
        }
    }

    /* ========== lwset_equals ========== */
    test_sub("subtest %d: equal empty sets", ++subnum);
    {
        lwset a = lwset_initunlim();
        lwset b = lwset_initunlim();
        test_validate(lwset_equals(&a, &b), "Empty sets must be equal");
    }

    test_sub("subtest %d: equal non‑empty sets", ++subnum);
    {
        lwset a = lwset_initunlim();
        lwset b = lwset_initunlim();
        a.value = b.value = (1ULL << 3) | (1ULL << 7);
        test_validate(lwset_equals(&a, &b), "Identical non‑empty sets must be equal");
    }

    test_sub("subtest %d: equal with different ranges (only value compared)", ++subnum);
    {
        lwset a = lwset_init0(0, 10);
        lwset b = lwset_init0(20, 30);
        a.value = b.value = 0xAA;
        // согласно реализации сравнивается только value, поэтому должно быть true
        test_validate(lwset_equals(&a, &b), "Sets with same value but different ranges must be equal");
    }

    test_sub("subtest %d: not equal", ++subnum);
    {
        lwset a = lwset_initunlim();
        lwset b = lwset_initunlim();
        a.value = 0xFF;
        b.value = 0xFE;
        test_validate(!lwset_equals(&a, &b), "Different values must not be equal");
    }

    /* ========== lwset_notequal (противоположность) ========== */
    test_sub("subtest %d: notequal basic", ++subnum);
    {
        lwset a = lwset_initunlim();
        lwset b = lwset_initunlim();
        a.value = 1; b.value = 2;
        test_validate(lwset_notequal(&a, &b), "Different sets must be notequal");
        test_validate(!lwset_notequal(&a, &a), "Same set must not be notequal");
    }

    return logret(TEST_PASSED, "done");
}

// ------------------------- TEST lwset_in / lwset_strictin / lwset_notempty / lwset_isempty -------------------------
static TestStatus
tf_lwset_in_strictin_empty(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    /* ========== lwset_in ========== */
    test_sub("subtest %d: empty in empty → true", ++subnum);
    {
        lwset a = lwset_initunlim();
        lwset b = lwset_initunlim();
        test_validate(lwset_in(&a, &b), "Empty set must be subset of empty set");
    }

    test_sub("subtest %d: empty in non‑empty → true", ++subnum);
    {
        lwset a = lwset_initunlim();
        lwset b = lwset_initunlim();
        b.value = 0xFF;
        test_validate(lwset_in(&a, &b), "Empty set must be subset of any set");
    }

    test_sub("subtest %d: non‑empty in empty → false", ++subnum);
    {
        lwset a = lwset_initunlim();
        lwset b = lwset_initunlim();
        a.value = 0xFF;
        test_validate(!lwset_in(&a, &b), "Non‑empty set must not be subset of empty set");
    }

    test_sub("subtest %d: proper subset → true", ++subnum);
    {
        lwset a = lwset_initunlim();
        lwset b = lwset_initunlim();
        a.value = (1ULL << 2);
        b.value = (1ULL << 2) | (1ULL << 5);
        test_validate(lwset_in(&a, &b), "Proper subset must be detected");
    }

    test_sub("subtest %d: equal sets → true", ++subnum);
    {
        lwset a = lwset_initunlim();
        lwset b = lwset_initunlim();
        a.value = b.value = 0xABCD;
        test_validate(lwset_in(&a, &b), "Equal sets are subsets of each other");
    }

    test_sub("subtest %d: non‑overlapping bits → false", ++subnum);
    {
        lwset a = lwset_initunlim();
        lwset b = lwset_initunlim();
        a.value = (1ULL << 3);
        b.value = (1ULL << 7);
        test_validate(!lwset_in(&a, &b), "Disjoint sets must not be subsets");
    }

    /* ========== lwset_strictin ========== */
    test_sub("subtest %d: empty strictin empty → false", ++subnum);
    {
        lwset a = lwset_initunlim();
        lwset b = lwset_initunlim();
        test_validate(!lwset_strictin(&a, &b), "Empty must not be strict subset of itself");
    }

    test_sub("subtest %d: empty strictin non‑empty → true", ++subnum);
    {
        lwset a = lwset_initunlim();
        lwset b = lwset_initunlim();
        b.value = 0xFF;
        test_validate(lwset_strictin(&a, &b), "Empty must be strict subset of non‑empty");
    }

    test_sub("subtest %d: non‑empty strictin empty → false", ++subnum);
    {
        lwset a = lwset_initunlim();
        lwset b = lwset_initunlim();
        a.value = 0xFF;
        test_validate(!lwset_strictin(&a, &b), "Non‑empty must not be strict subset of empty");
    }

    test_sub("subtest %d: proper subset strictin → true", ++subnum);
    {
        lwset a = lwset_initunlim();
        lwset b = lwset_initunlim();
        a.value = (1ULL << 2);
        b.value = (1ULL << 2) | (1ULL << 5);
        test_validate(lwset_strictin(&a, &b), "Proper subset must be strict");
    }

    test_sub("subtest %d: equal sets strictin → false", ++subnum);
    {
        lwset a = lwset_initunlim();
        lwset b = lwset_initunlim();
        a.value = b.value = 0xABCD;
        test_validate(!lwset_strictin(&a, &b), "Equal sets must not be strict subsets");
    }

    /* ========== lwset_notempty / lwset_isempty ========== */
    test_sub("subtest %d: empty set is empty", ++subnum);
    {
        lwset s = lwset_initunlim();
        test_validate(lwset_isempty(&s), "Empty set must be empty");
        test_validate(!lwset_notempty(&s), "Empty set must not be non‑empty");
    }

    test_sub("subtest %d: non‑empty set is not empty", ++subnum);
    {
        lwset s = lwset_initunlim();
        s.value = 0x1;
        test_validate(!lwset_isempty(&s), "Non‑empty must not be empty");
        test_validate(lwset_notempty(&s), "Non‑empty must be non‑empty");
    }

    /* ========== NULL checks ========== */
    test_sub("subtest %d: lwset_in with NULL raises", ++subnum);
    {
        lwset s = lwset_initunlim();
        if (!try()) {
            lwset_in(NULL, &s);
            test_validate(false, "Should have raised SIGINT for NULL s1");
        } else {
            logsimple("Exception correctly raised for NULL s1");
        }
        if (!try()) {
            lwset_in(&s, NULL);
            test_validate(false, "Should have raised SIGINT for NULL s2");
        } else {
            logsimple("Exception correctly raised for NULL s2");
        }
    }

    test_sub("subtest %d: lwset_isempty(NULL) raises", ++subnum);
    {
        if (!try()) {
            lwset_isempty(NULL);
            test_validate(false, "Should have raised SIGINT");
        } else {
            logsimple("Exception correctly raised for lwset_isempty(NULL)");
        }
    }

    test_sub("subtest %d: lwset_notempty(NULL) raises", ++subnum);
    {
        if (!try()) {
            lwset_notempty(NULL);
            test_validate(false, "Should have raised SIGINT");
        } else {
            logsimple("Exception correctly raised for lwset_notempty(NULL)");
        }
    }

    test_sub("subtest %d: lwset_strictin with NULL raises", ++subnum);
    {
        lwset s = lwset_initunlim();
        if (!try()) {
            lwset_strictin(NULL, &s);
            test_validate(false, "Should have raised SIGINT for NULL s1");
        } else {
            logsimple("Exception correctly raised for NULL s1");
        }
        if (!try()) {
            lwset_strictin(&s, NULL);
            test_validate(false, "Should have raised SIGINT for NULL s2");
        } else {
            logsimple("Exception correctly raised for NULL s2");
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
                testnew(.f2 =  tf_lwset_init,        .num =  1, .name = "Lwset init simple test", 
                        .desc="", .mandatory=true)
              , testnew(.f2 =  tf_lwset_clone_list,  .num =  2, .name = "Lwset clone and list test", 
                        .desc="", .mandatory=true)
              , testnew(.f2 =  tf_lwset_get_equals,  .num =  3, .name = "Lwset get/equals/notequal test",
                        .desc="", .mandatory=true)
              , testnew(.f2 =  tf_lwset_in_strictin_empty,  .num =  4, .name = "Lwset in/strictin/empty test",
                        .desc="", .mandatory=true)
              );
        if (runall)
            break;
    }

    return logret(0, "end...");  // as replace of logclose()
}

#endif /* LWSET_TESTING */
