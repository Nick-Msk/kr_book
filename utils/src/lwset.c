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

bool                    lwset_isvalid(const lwset * s) {
    bool res = s != NULL;
    if (!res)
        return userraise(false, ERR_NULLABLE_PTR, "Pointer is NULL");
    invraisecode(ERR_NULLABLE_PTR, s != NULL, "Pointer is NULL");
    res = s->low <= s->high && s->high < LWSET_MAX_BITS;  // ensure the range is valid and within bounds
    if (!res)
        return userraise(false, ERR_OUT_OF_RANGE, "Invalid lwset range: low=%u, high=%u", s->low, s->high);
    for (unsigned short i = 0; i < LWSET_MAX_BITS; ++i) {
        // check if the bits outside the range are not set
        if ( ( (s->value >> i) & 1 ) && (i < s->low || i > s->high) ) {
            return userraise(false, ERR_OUT_OF_RANGE, "Invalid lwset: bit %u is set outside the range [%u, %u]", i, s->low, s->high);
        }
    }
    return logsimpleret(res, "Validated!");
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
        test_validate(
            lwset_isvalid(&s), 
            "Set must be valid after initialization"
        );
    }

    test_sub("subtest %d: lwset_init1unlim creates full set [0,63]", ++subnum);
    {
        lwset s = lwset_init1unlim();
        test_validate(
            s.value == UINT64_MAX && s.low == 0 && s.high == 63,
            "init1unlim: value=0x%016llx (expected all ones)", (unsigned long long)s.value
        );
        test_validate(
            lwset_isvalid(&s),
            "Set must be valid after initialization"
        );
    }

    test_sub("subtest %d: lwset_init0 with range [10,20]", ++subnum);
    {
        lwset s = lwset_init0(10, 20);
        test_validate(
            s.value == 0 && s.low == 10 && s.high == 20,
            "init0 range: value=0x%llx, low=%u, high=%u", (unsigned long long)s.value, s.low, s.high
        );
        test_validate(
            lwset_isvalid(&s),
            "Set must be valid after initialization"
        );
    }

    test_sub("subtest %d: lwset_init1 with range [5,10]", ++subnum);
    {
        lwset s = lwset_init1(5, 10);
        uint64_t expected = ((1ULL << (10 - 5 + 1)) - 1) << 5;  // 0x3F << 5 = 0x7E0
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
        test_validate(
            lwset_isvalid(&empty),
            "Empty set must be valid"
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
        test_validate(
            lwset_isvalid(&copy),
            "Cloned set must be valid"
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
        test_validate(
            lwset_isvalid(&copy),
            "Cloned set must be valid"
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
        test_validate(
            lwset_isvalid(&s),
            "Set must be valid after list initialization"
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
        test_validate(
            lwset_isvalid(&s),
            "Set must be valid after list initialization"
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
        test_validate(
            lwset_isvalid(&s),
            "Set must be valid after list duplicate initialization"
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
        test_validate(
            lwset_isvalid(&s),
            "Set must be valid after LWSET_LIST initialization"
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
        test_validate(
            lwset_in(&a, &b), 
            "Empty set must be subset of empty set"
        );
    }

    test_sub("subtest %d: empty in non‑empty → true", ++subnum);
    {
        lwset a = lwset_initunlim();
        lwset b = lwset_initunlim();
        b.value = 0xFF;
        test_validate(
            lwset_in(&a, &b), 
            "Empty set must be subset of any set");
    }

    test_sub("subtest %d: non‑empty in empty → false", ++subnum);
    {
        lwset a = lwset_initunlim();
        lwset b = lwset_initunlim();
        a.value = 0xFF;
        test_validate(
            !lwset_in(&a, &b), 
            "Non‑empty set must not be subset of empty set"
        );
    }

    test_sub("subtest %d: proper subset → true", ++subnum);
    {
        lwset a = lwset_initunlim();
        lwset b = lwset_initunlim();
        a.value = (1ULL << 2);
        b.value = (1ULL << 2) | (1ULL << 5);
        test_validate(
            lwset_in(&a, &b), 
            "Proper subset must be detected"
        );
    }

    test_sub("subtest %d: equal sets → true", ++subnum);
    {
        lwset a = lwset_initunlim();
        lwset b = lwset_initunlim();
        a.value = b.value = 0xABCD;
        test_validate(
            lwset_in(&a, &b), 
            "Equal sets are subsets of each other"
        );
    }

    test_sub("subtest %d: non‑overlapping bits → false", ++subnum);
    {
        lwset a = lwset_initunlim();
        lwset b = lwset_initunlim();
        a.value = (1ULL << 3);
        b.value = (1ULL << 7);
        test_validate(
            !lwset_in(&a, &b), 
            "Disjoint sets must not be subsets"
        );
    }

    /* ========== lwset_strictin ========== */
    test_sub("subtest %d: empty strictin empty → false", ++subnum);
    {
        lwset a = lwset_initunlim();
        lwset b = lwset_initunlim();
        test_validate(
            !lwset_strictin(&a, &b), 
            "Empty must not be strict subset of itself"
        );
    }

    test_sub("subtest %d: empty strictin non‑empty → true", ++subnum);
    {
        lwset a = lwset_initunlim();
        lwset b = lwset_initunlim();
        b.value = 0xFF;
        test_validate(
            lwset_strictin(&a, &b), 
            "Empty must be strict subset of non‑empty"
        );
    }

    test_sub("subtest %d: non‑empty strictin empty → false", ++subnum);
    {
        lwset a = lwset_initunlim();
        lwset b = lwset_initunlim();
        a.value = 0xFF;
        test_validate(
            !lwset_strictin(&a, &b), 
            "Non‑empty must not be strict subset of empty"
        );
    }

    test_sub("subtest %d: proper subset strictin → true", ++subnum);
    {
        lwset a = lwset_initunlim();
        lwset b = lwset_initunlim();
        a.value = (1ULL << 2);
        b.value = (1ULL << 2) | (1ULL << 5);
        test_validate(
            lwset_strictin(&a, &b), 
            "Proper subset must be strict"
        );
    }

    test_sub("subtest %d: equal sets strictin → false", ++subnum);
    {
        lwset a = lwset_initunlim();
        lwset b = lwset_initunlim();
        a.value = b.value = 0xABCD;
        test_validate(
            !lwset_strictin(&a, &b), 
            "Equal sets must not be strict subsets"
        );
    }

    /* ========== lwset_notempty / lwset_isempty ========== */
    test_sub("subtest %d: empty set is empty", ++subnum);
    {
        lwset s = lwset_initunlim();
        test_validate(
            lwset_isempty(&s), 
            "Empty set must be empty"
        );
        test_validate(
            !lwset_notempty(&s), 
            "Empty set must not be non‑empty"
        );
    }

    test_sub("subtest %d: non‑empty set is not empty", ++subnum);
    {
        lwset s = lwset_initunlim();
        s.value = 0x1;
        test_validate(
            !lwset_isempty(&s), 
            "Non‑empty must not be empty"
        );
        test_validate(
            lwset_notempty(&s), 
            "Non‑empty must be non‑empty"
        );
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

// ------------------------- TEST lwset_isvalid -------------------------
static TestStatus
tf_lwset_isvalid(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    /* 1. Valid empty set with default range */
    test_sub("subtest %d: valid empty set [0,63]", ++subnum);
    {
        lwset s = lwset_initunlim();
        test_validate(
            lwset_isvalid(&s), 
            "Empty set [0,63] must be valid"
        );
    }

    /* 2. Valid set with bits inside range */
    test_sub("subtest %d: valid set with bits in range", ++subnum);
    {
        lwset s = lwset_init0(5, 20);
        s.value = (1ULL << 10) | (1ULL << 15);
        test_validate(
            lwset_isvalid(&s), 
            "Bits inside [5,20] must be valid"
        );
    }

    /* 3. Valid set with bits at the boundaries */
    test_sub("subtest %d: valid set with bits at low and high", ++subnum);
    {
        lwset s = lwset_init0(10, 30);
        s.value = (1ULL << 10) | (1ULL << 30);
        test_validate(
            lwset_isvalid(&s), 
            "Bits at low=10 and high=30 must be valid"
        );
    }

    /* 4. low > high → invalid */
    test_sub("subtest %d: low > high makes it invalid", ++subnum);
    {
        lwset s = { .value = 0, .low = 20, .high = 10 };
        if (!try()) {
            bool valid = lwset_isvalid(&s);
            test_validate(
                !valid, 
                "Set with low>high must return false"
            );
        } else {
            logsimple("lwset_isvalid raised exception for low>high (expected)");
        }
    }

    /* 5. high >= LWSET_MAX_BITS → invalid */
    test_sub("subtest %d: high >= LWSET_MAX_BITS is invalid", ++subnum);
    {
        lwset s = { .value = 0, .low = 0, .high = 64 }; // assuming LWSET_MAX_BITS=64
        if (!try()) {
            bool valid = lwset_isvalid(&s);
            test_validate(
                !valid, 
                "Set with high>=64 must return false"
            );
        } else {
            logsimple("lwset_isvalid raised exception for high>=64 (expected)");
        }
    }

    /* 6. Bit set outside range → invalid */
    test_sub("subtest %d: bit outside range makes it invalid", ++subnum);
    {
        lwset s = lwset_init0(10, 20);
        s.value = (1ULL << 5);   // bit 5 < low=10
        if (!try()) {
            bool valid = lwset_isvalid(&s);
            test_validate(
                !valid, 
                "Bit 5 outside [10,20] must be invalid"
            );
        } else {
            logsimple("lwset_isvalid raised exception for bit outside range (expected)");
        }
    }

    /* 7. Another out-of-range bit above high */
    test_sub("subtest %d: bit above high makes it invalid", ++subnum);
    {
        lwset s = lwset_init0(0, 40);
        s.value = (1ULL << 50);   // 50 > 40
        if (!try()) {
            bool valid = lwset_isvalid(&s);
            test_validate(
                !valid, 
                "Bit 50 outside [0,40] must be invalid"
            );
        } else {
            logsimple("lwset_isvalid raised exception for bit above high (expected)");
        }
    }

    /* 8. NULL pointer raises SIGINT */
    test_sub("subtest %d: NULL pointer raises SIGINT", ++subnum);
    {
        if (!try()) {
            lwset_isvalid(NULL);
            test_validate(false, "Should have raised SIGINT for NULL");
        } else {
            logsimple("Exception correctly raised for NULL");
        }
    }

    return logret(TEST_PASSED, "done");
}

// ------------------------- TEST lwset_set / lwset_unset / lwset_setrangevalue -------------------------
static TestStatus
tf_lwset_set_unset_range(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    /* ========== lwset_set ========== */
    test_sub("subtest %d: set bit in empty set", ++subnum);
    {
        lwset s = lwset_initunlim();
        lwset *res = lwset_set(&s, 5);
        test_validate(
            res == &s && lwset_get(&s, 5) == true,
            "Set bit 5 must be true, res must be &s"
        );
        test_validate(
            s.value == (1ULL << 5),
            "Only bit 5 must be set, value=0x%llx", (uint64_t) s.value
        );
        test_validate(
            lwset_isvalid(&s),
            "Set must remain valid after setting a bit"
        );
    }

    test_sub("subtest %d: set bit at low boundary", ++subnum);
    {
        lwset s = lwset_init0(10, 30);
        lwset_set(&s, 10);
        test_validate(
            lwset_get(&s, 10) == true,
            "Bit at low=10 must be set"
        );
        test_validate(
            lwset_isvalid(&s), 
            "Set must remain valid after setting a bit"
        );
    }

    test_sub("subtest %d: set bit at high boundary", ++subnum);
    {
        lwset s = lwset_init0(10, 30);
        lwset_set(&s, 30);
        test_validate(
            lwset_get(&s, 30) == true, 
            "Bit at high=30 must be set"
        );
        test_validate(
            lwset_isvalid(&s), 
            "Set must remain valid after setting a bit"
        );
    }

    test_sub("subtest %d: set bit out of range raises", ++subnum);
    {
        lwset s = lwset_init0(10, 20);
        if (!try()) {
            lwset_set(&s, 5);
            test_validate(false, "Should have raised SIGINT for index<low");
        } else {
            logsimple("Exception correctly raised for index<low");
        }
        test_validate(
            lwset_isvalid(&s), 
            "Set must remain valid after setting a bit"
        );
        if (!try()) {
            lwset_set(&s, 25);
            test_validate(false, "Should have raised SIGINT for index>high");
        } else {
            logsimple("Exception correctly raised for index>high");
        }
    }

    /* ========== lwset_unset ========== */
    test_sub("subtest %d: unset a set bit", ++subnum);
    {
        lwset s = lwset_initunlim();
        lwset_set(&s, 7);
        lwset_unset(&s, 7);
        test_validate(
            lwset_get(&s, 7) == false, 
            "Bit 7 must be cleared"
        );
        test_validate(
            s.value == 0, 
            "Value must be 0 after unset"
        );
        test_validate(
            lwset_isvalid(&s), 
            "Set must remain valid after unsetting a bit"
        );
    }

    test_sub("subtest %d: unset already cleared bit", ++subnum);
    {
        lwset s = lwset_initunlim();
        lwset_unset(&s, 3);
        test_validate(
            lwset_get(&s, 3) == false, 
            "Unset of unset bit keeps it false"
        );
        test_validate(
            lwset_isvalid(&s), 
            "Set must remain valid after unsetting a bit"
        );
    }

    test_sub("subtest %d: unset out of range raises", ++subnum);
    {
        lwset s = lwset_init0(0, 63);
        if (!try()) {
            lwset_unset(&s, 64);
            test_validate(false, "Should have raised SIGINT for index>63");
        } else {
            logsimple("Exception correctly raised for index>63");
        }
        test_validate(
            lwset_isvalid(&s), 
            "Set must remain valid after unsetting a bit"
        );
    }

    /* ========== lwset_setrangevalue ========== */
    test_sub("subtest %d: set range of bits to true", ++subnum);
    {
        lwset s = lwset_init0(0, 20);
        lwset_setrangevalue(&s, 2, 5, true);
        for (unsigned short i = 2; i <= 5; ++i)
            test_validate(
                lwset_get(&s, i) == true, 
                "Bit %u must be set", i
            );
        test_validate(
            lwset_get(&s, 1) == false && lwset_get(&s, 6) == false,
            "Bits outside range must remain 0"
        );
        test_validate(
            lwset_isvalid(&s), 
            "Set must remain valid after setting a range"
        );
    }

    test_sub("subtest %d: clear range of bits (set to false)", ++subnum);
    {
        lwset s = lwset_init0(0, 20);
        lwset_setrangevalue(&s, 0, 19, true);
        lwset_setrangevalue(&s, 5, 10, false);
        for (unsigned short i = 5; i <= 10; ++i)
            test_validate(
                lwset_get(&s, i) == false, 
                "Bit %u must be cleared", i
            );
        test_validate(
            lwset_get(&s, 0) == true && lwset_get(&s, 4) == true &&
            lwset_get(&s, 11) == true, 
            "Bits outside cleared range must stay set"
        );
        test_validate(
            lwset_isvalid(&s), 
            "Set must remain valid after setting a range"
        );
    }

    test_sub("subtest %d: setrange with invalid range (low > high) raises", ++subnum);
    {
        lwset s = lwset_init0(0, 30);
        if (!try()) {
            lwset_setrangevalue(&s, 10, 5, true);
            test_validate(false, "Should have raised SIGINT for low>high");
        } else {
            logsimple("Exception correctly raised for low>high");
        }
        test_validate(
            lwset_isvalid(&s), 
            "Set must remain valid after setting a range"
        );
    }

    test_sub("subtest %d: setrange with low < s->low raises", ++subnum);
    {
        lwset s = lwset_init0(10, 20);
        if (!try()) {
            lwset_setrangevalue(&s, 5, 15, true);
            test_validate(false, "Should have raised SIGINT for low< s->low");
        } else {
            logsimple("Exception correctly raised for low< s->low");
        }
        test_validate(
            lwset_isvalid(&s), 
            "Set must remain valid after setting a range"
        );
    }

    test_sub("subtest %d: setrange with high > s->high raises", ++subnum);
    {
        lwset s = lwset_init0(10, 20);
        if (!try()) {
            lwset_setrangevalue(&s, 15, 25, true);
            test_validate(false, "Should have raised SIGINT for high> s->high");
        } else {
            logsimple("Exception correctly raised for high> s->high");
        }
        test_validate(
            lwset_isvalid(&s), 
            "Set must remain valid after setting a range"
        );
    }

    test_sub("subtest %d: NULL pointer raises", ++subnum);
    {
        if (!try()) {
            lwset_set(NULL, 0);
            test_validate(false, "Should have raised SIGINT for NULL");
        } else {
            logsimple("Exception correctly raised for NULL in lwset_set");
        }
        if (!try()) {
            lwset_unset(NULL, 0);
            test_validate(false, "Should have raised SIGINT for NULL");
        } else {
            logsimple("Exception correctly raised for NULL in lwset_unset");
        }
        if (!try()) {
            lwset_setrangevalue(NULL, 0, 10, true);
            test_validate(false, "Should have raised SIGINT for NULL");
        } else {
            logsimple("Exception correctly raised for NULL in lwset_setrangevalue");
        }
    }

    return logret(TEST_PASSED, "done");
}

// ------------------------- TEST lwset_union / lwset_intersect / lwset_minus / lwset_symmdiff -------------------------
static TestStatus
tf_lwset_ops(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    /* ========== lwset_union ========== */
    test_sub("subtest %d: union of two non‑empty sets", ++subnum);
    {
        lwset a = lwset_initunlim();
        lwset b = lwset_initunlim();
        a.value = (1ULL << 2) | (1ULL << 5);
        b.value = (1ULL << 5) | (1ULL << 7);
        lwset *res = lwset_union(&a, &b);
        test_validate(
            res == &a &&
            a.value == ((1ULL << 2) | (1ULL << 5) | (1ULL << 7)),
            "Union must contain bits 2,5,7"
        );
        test_validate(
            lwset_isvalid(&a), 
            "Set must be valid after union"
        );
    }

    test_sub("subtest %d: union with empty set", ++subnum);
    {
        lwset a = lwset_initunlim();
        lwset b = lwset_initunlim();
        a.value = 0xAA;
        lwset_union(&a, &b);
        test_validate(
            a.value == 0xAA, 
            "Union with empty set must keep a unchanged"
        );
        test_validate(
            lwset_isvalid(&a), 
            "Set must be valid after union with empty"
        );
    }

    test_sub("subtest %d: union with NULL raises", ++subnum);
    {
        lwset a = lwset_initunlim();
        if (!try()) {
            lwset_union(NULL, &a);
            test_validate(false, "Should have raised SIGINT");
        } else {
            logsimple("Exception on NULL s1");
        }
        if (!try()) {
            lwset_union(&a, NULL);
            test_validate(false, "Should have raised SIGINT");
        } else {
            logsimple("Exception on NULL s2");
        }
    }

    /* ========== lwset_intersect ========== */
    test_sub("subtest %d: intersect of overlapping sets", ++subnum);
    {
        lwset a = lwset_initunlim();
        lwset b = lwset_initunlim();
        a.value = (1ULL << 3) | (1ULL << 6) | (1ULL << 9);
        b.value = (1ULL << 6) | (1ULL << 9) | (1ULL << 12);
        lwset_intersect(&a, &b);
        test_validate(
            a.value == ((1ULL << 6) | (1ULL << 9)),
            "Intersect must keep only bits 6,9"
        );
        test_validate(
            lwset_isvalid(&a), 
            "Set must be valid after intersect"
        );
    }

    test_sub("subtest %d: intersect with empty set", ++subnum);
    {
        lwset a = lwset_initunlim();
        lwset b = lwset_initunlim();
        a.value = 0xFF;
        lwset_intersect(&a, &b);
        test_validate(
            a.value == 0, 
            "Intersect with empty set must clear a"
        );
        test_validate(
            lwset_isvalid(&a), 
            "Set must be valid after intersect with empty"
        );
    }

    test_sub("subtest %d: intersect with NULL raises", ++subnum);
    {
        lwset a = lwset_initunlim();
        if (!try()) { 
            lwset_intersect(NULL, &a); 
            test_validate(false, "NULL s1"); 
        } else
            logsimple("Exception on NULL s1");
        if (!try()) { 
            lwset_intersect(&a, NULL); 
            test_validate(false, "NULL s2"); 
        }
        else 
            logsimple("Exception on NULL s2");
    }

    /* ========== lwset_minus ========== */
    test_sub("subtest %d: minus of overlapping sets", ++subnum);
    {
        lwset a = lwset_initunlim();
        lwset b = lwset_initunlim();
        a.value = (1ULL << 2) | (1ULL << 4) | (1ULL << 6);
        b.value = (1ULL << 4);
        lwset_minus(&a, &b);
        test_validate(
            a.value == ((1ULL << 2) | (1ULL << 6)),
            "Minus must remove bit 4"
        );
        test_validate(
            lwset_isvalid(&a), 
            "Set must be valid after minus"
        );
    }

    test_sub("subtest %d: minus with empty set", ++subnum);
    {
        lwset a = lwset_initunlim();
        lwset b = lwset_initunlim();
        a.value = 0xAA;
        lwset_minus(&a, &b);
        test_validate(
            a.value == 0xAA, 
            "Minus with empty set keeps a unchanged"
        );
        test_validate(
            lwset_isvalid(&a), 
            "Set must be valid after minus with empty"
        );
    }

    test_sub("subtest %d: minus with NULL raises", ++subnum);
    {
        lwset a = lwset_initunlim();
        if (!try()) { 
            lwset_minus(NULL, &a); 
            test_validate(false, "NULL s1"); 
        } else 
            logsimple("Exception on NULL s1");
        if (!try()) { 
            lwset_minus(&a, NULL); 
            test_validate(false, "NULL s2"); 
        } else
            logsimple("Exception on NULL s2");
    }

    /* ========== lwset_symmdiff ========== */
    test_sub("subtest %d: symmetric difference of overlapping sets", ++subnum);
    {
        lwset a = lwset_initunlim();
        lwset b = lwset_initunlim();
        a.value = (1ULL << 1) | (1ULL << 3) | (1ULL << 5);
        b.value = (1ULL << 3) | (1ULL << 5) | (1ULL << 7);
        lwset_symmdiff(&a, &b);
        test_validate(
            a.value == ((1ULL << 1) | (1ULL << 7)),
            "Symmetric difference must leave bits 1 and 7"
        );
        test_validate(
            lwset_isvalid(&a), 
            "Set must be valid after symmdiff"
        );
    }

    test_sub("subtest %d: symmetric difference with empty set", ++subnum);
    {
        lwset a = lwset_initunlim();
        lwset b = lwset_initunlim();
        a.value = 0x55;
        lwset_symmdiff(&a, &b);
        test_validate(
            a.value == 0x55, 
            "Symmetric diff with empty set keeps a unchanged"
        );
        test_validate(
            lwset_isvalid(&a), 
            "Set must be valid after symmdiff with empty"
        );
    }

    test_sub("subtest %d: symmetric difference of equal sets is empty", ++subnum);
    {
        lwset a = lwset_initunlim();
        lwset b = lwset_initunlim();
        a.value = b.value = 0x1234;
        lwset_symmdiff(&a, &b);
        test_validate(
            a.value == 0, 
            "Symmetric diff of identical sets must be 0"
        );
        test_validate(
            lwset_isvalid(&a), 
            "Set must be valid after symmdiff of equals"
        );
    }

    test_sub("subtest %d: symmdiff with NULL raises", ++subnum);
    {
        lwset a = lwset_initunlim();
        if (!try()) { 
            lwset_symmdiff(NULL, &a); 
            test_validate(false, "NULL s1"); 
        } else
            logsimple("Exception on NULL s1");
        if (!try()) { 
            lwset_symmdiff(&a, NULL); 
            test_validate(false, "NULL s2");
        } else
            logsimple("Exception on NULL s2");
    }

    return logret(TEST_PASSED, "done");
}

// ------------------------- TEST lwset_count -------------------------
static TestStatus
tf_lwset_count(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    test_sub("subtest %d: count empty set", ++subnum);
    {
        lwset s = lwset_initunlim();
        test_validate(
            lwset_count(&s) == 0,
            "Empty set must have count 0"
        );
    }

    test_sub("subtest %d: count single bit", ++subnum);
    {
        lwset s = lwset_initunlim();
        lwset_set(&s, 10);
        test_validate(
            lwset_count(&s) == 1,
            "Single bit set must count 1"
        );
    }

    test_sub("subtest %d: count multiple bits", ++subnum);
    {
        lwset s = lwset_initunlim();
        lwset_set(&s, 2);
        lwset_set(&s, 5);
        lwset_set(&s, 63);
        test_validate(
            lwset_count(&s) == 3,
            "Three bits set must count 3"
        );
    }

    test_sub("subtest %d: count full set (all bits)", ++subnum);
    {
        lwset s = lwset_init1unlim();   // все 64 бита
        test_validate(
            lwset_count(&s) == 64,
            "Full set must count 64"
        );
    }

    test_sub("subtest %d: count only within range", ++subnum);
    {
        lwset s = lwset_init1(5, 10);   // биты 5..10 установлены, всего 6
        test_validate(
            lwset_count(&s) == 6,
            "Range [5,10] full must count 6"
        );
    }

    test_sub("subtest %d: count with modified bits inside range", ++subnum);
    {
        lwset s = lwset_init0(0, 20);
        lwset_set(&s, 5);
        lwset_set(&s, 15);
        lwset_set(&s, 0);
        test_validate(
            lwset_count(&s) == 3,
            "Three bits inside range must count 3"
        );
        test_validate(
            lwset_isvalid(&s), 
            "Set must be valid"
        );
    }

    test_sub("subtest %d: count NULL raises", ++subnum);
    {
        if (!try()) {
            lwset_count(NULL);
            test_validate(false, "Should have raised SIGINT for NULL");
        } else {
            logsimple("Exception correctly raised for NULL");
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
            testnew(.f2 =  tf_lwset_init,                 .num =  1, .name = "Lwset init simple test", 
                    .desc="", .mandatory=true)
            , testnew(.f2 =  tf_lwset_clone_list,         .num =  2, .name = "Lwset clone and list test", 
                    .desc="", .mandatory=true)
            , testnew(.f2 =  tf_lwset_get_equals,         .num =  3, .name = "Lwset get/equals/notequal test",
                    .desc="", .mandatory=true)
            , testnew(.f2 =  tf_lwset_in_strictin_empty,  .num =  4, .name = "Lwset in/strictin/empty test",
                    .desc="", .mandatory=true)
            , testnew(.f2 =  tf_lwset_isvalid,            .num =  5, .name = "Lwset isvalid test",
                    .desc="", .mandatory=true)
            , testnew(.f2 =  tf_lwset_set_unset_range,    .num =  6, .name = "Lwset set/unset/range test",
                    .desc="", .mandatory=true)
            , testnew(.f2 =  tf_lwset_ops,                .num =  7, .name = "Lwset operations test",
                    .desc="", .mandatory=true)
            , testnew(.f2 =  tf_lwset_count,              .num =  8, .name = "Lwset count test",
                    .desc="", .mandatory=true)
            );
        if (runall)
            break;
    }

    return logret(0, "end...");  // as replace of logclose()
}

#endif /* LWSET_TESTING */
