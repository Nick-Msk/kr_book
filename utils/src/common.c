#include "common.h"
#include "log.h"
#include <sys/errno.h>

/********************************************************************
                 COOMON MODULE IMPLEMENTATION
********************************************************************/

// --------------------------- API ---------------------------------

// int[] filler     // TODO: probably to use common code for all of that
void                            fill_int(int *arr, int cnt, int value){ 
    for (int i = 0; i < cnt; i++)
        arr[i] = value;
}

// long[] filler
void                            fill_long(long *arr, int cnt, long value){
    for (int i = 0; i < cnt; i++)
        arr[i] = value;
}

// double[] filler
void                            fill_double(double *arr, int cnt, double value){
    for (int i = 0; i < cnt; i++)
        arr[i] = value;
}

// float[] filler
void                            fill_float(float *arr, int cnt, float value){
    for (int i = 0; i < cnt; i++)
        arr[i] = value;
}

char                            *uniq_str(char *s, int *p_len){
    bool    hash[256] = {false};
    int     j = 0, i = 0;
    char    c;
    while ( (c = s[i++]) != '\0'){
        if (!hash[(int) c]){
            hash[(int) c] = true;
            s[j++] = c;
        }
    }
    s[j] = '\0';
    if (p_len)
        *p_len = j;
    return s; // logret(s, "new len = %d, new str[%s]", j, s);
}

// bits to string (STATIC for now) TODO: think about FastString usage here
extern const char               *bits_str(char *buf, int len, unsigned val){
    //logenter("val %u", val);
    int        pos = 0, bit;
    while (pos < len - 1 && val > 0){ 
        bit = val & 0x1;
        buf[pos++] = bit + '0';
        val >>= 1;
    }
    buf[pos] = '\0';
    reverse(buf, pos);
    return buf; //logret(buf, "[%s]", buf);
}

int                             fprint_bits(FILE *f, const char *str, unsigned val){
    char            buf[100];
    bits_str(buf, sizeof(buf), val);
    return fprintf(f, "%s: %s\n", str, buf);
}

// reverse string
char*                           reverse(char *s, int len){
    int i = 0, j = len - 1;
    while (i < j)
        char_exch(s + i++, s + j--);
    return s;
}

int                             fprintn(FILE *f, const char *str, int sz){
    char c;
    int i = 0;
    while (str && i < sz &&(c = str[i++]) != '\0')
        fputc(c, f);
    fputc('\n', f);
    return i;
}

// simple comparator pointer double
int                             pdbl_cmp(const void *d1, const void *d2){
    if ( *(double *) d1 > *(double *) d2)
        return 1;
    else if ( *(double *) d1 < *(double *) d2)
        return -1;
    else
        return 0;
}

// simple reverse comparator pointer double
int                             pdbl_revcmp(const void *d1, const void *d2){
    if ( *(double *) d1 < *(double *) d2)
        return 1;
    else if ( *(double *) d1 > *(double *) d2)
        return -1;
    else
        return 0;
}

static inline bool              is_long_overflow(long val){
    return errno == ERANGE && (val == LONG_MAX || val == LONG_MIN);
}
static inline bool              is_ulong_overflow(unsigned long val){
    return errno == ERANGE && val == ULONG_MAX;
}
static inline bool              is_double_overflow(double val){
    return errno == ERANGE && isinf(val);
}

bool                            try_parse_int(const char *restrict str, int *restrict res) {
    if (!str)
        return logsimpleerr(false, "Null pointer str");
    errno = 0;
    char    *endptr;
    long val = strtol(str, &endptr, 10);

    if (str == endptr || *endptr != '\0' || val > INT_MAX || val < INT_MIN)
        return logsimpleerr(false, "Unable to parse int %ld, errno %d", val, errno);
    if (res)
        *res = (int) val;
    return true;
}
bool                            try_parse_long(const char *restrict str, long *restrict res) {
    if (!str)
        return logsimpleerr(false, "Null pointer str");
    errno = 0;
    char    *endptr;
    long val = strtol(str, &endptr, 10);

    if (str == endptr || *endptr != '\0' || is_long_overflow(val) )
        return logsimpleerr(false, "Unable to parse long %ld, errno %d", val, errno);
    if (res)
        *res = val;
    return true;
}
bool                            try_parse_double(const char *restrict str, double *restrict res) {
    if (!str)
        return logsimpleerr(false, "Null pointer str");
    errno = 0;
    char    *endptr;
    double val = strtold(str, &endptr);
    if (str == endptr || *endptr != '\0' || is_double_overflow(val) ) {
        return logsimpleerr(false, "Unable to parse double %lg, errno %d", val, errno);
    }
    if (res)
        *res = val;
    return true;
}
bool                            try_parse_uint(const char *restrict str, unsigned *restrict res) {
    if (!str)
        return logsimpleerr(false, "Null pointer str");
    str = skip_leading_spaces(str);
    if (*str == '-')
        return logsimpleerr(false, "Negative value not allowed");
    errno = 0;
    char           *endptr;
    unsigned long   val = strtoul(str, &endptr, 10);

    if (str == endptr || *endptr != '\0' || *str == '-' || is_ulong_overflow(val) || val > UINT_MAX) {
        return logsimpleerr(false, "Unable to parse uint %lu, errno %d", val, errno);
    }

    if (res)
        *res = (unsigned int)val;
    return true;
}

bool                            try_parse_ulong(const char *restrict str, unsigned long *restrict res) {
    if (!str)
        return logsimpleerr(false, "Null pointer str");
    str = skip_leading_spaces(str);
    if (*str == '-')
        return logsimpleerr(false, "Negative value not allowed");
    errno = 0;
    char            *endptr;
    unsigned long    val = strtoul(str, &endptr, 10);

    if (str == endptr || *endptr != '\0' || *str == '-' || is_ulong_overflow(val))
        return logsimpleerr(false, "Unable to parse ulong %lu, errno %d", val, errno);

    if (res)
        *res = val;
    return true;
}
// -------------------------------Testing --------------------------

#ifdef COMMONTESTING

//#include <signal.h>
#include "test.h"
#include "checker.h"

// ------------------------- TEST try_parse ---------------------------------

static TestStatus
tf_try_parse(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    /* ========== try_parse_int ========== */

    /* 1. Успешный парсинг обычного int */
    test_sub("subtest %d: try_parse_int valid", ++subnum);
    {
        const char     *valid[] = {"0", "123", "-456", "2147483647", "-2147483648"};
        int             expected[] = {0, 123, -456, 2147483647, -2147483648};
        for (int i = 0; i < COUNT(valid); i++) {
            int     result;
            bool    ok;
            test_validate(
                (ok = try_parse_int(valid[i], &result) ) && result == expected[i],
                "try_parse_int('%s'): expected %d, got %d (ok=%s)",
                valid[i], expected[i], result, bool_str(ok));
        }
    }

    /* 2. Неудачи: пустая строка, буквы, частичный парсинг, переполнение, пробелы */
    test_sub("subtest %d: try_parse_int invalid", ++subnum);
    {
        const char *invalid[] = {
            "", "abc", "123abc", /* "  123", */ "123  ", "9999999999", "-9999999999"
        };
        for (int i = 0; i < COUNT(invalid); i++) {
            int     result;
            bool    ok;
            test_validate(
                !(ok = try_parse_int(invalid[i], &result) ),
                "try_parse_int('%s'): must fail, got ok=%s, result=%d",
                invalid[i], bool_str(ok), result);
        }
    }

    /* 3. NULL-указатель */
    test_sub("subtest %d: try_parse_int NULL", ++subnum);
    {
        int     result;
        bool    ok;
        test_validate(
            !(ok = try_parse_int(NULL, &result) ),
            "try_parse_int(NULL) must return false");
    }

    /* ========== try_parse_long ========== */

    /* 4. Успешный парсинг long */
    test_sub("subtest %d: try_parse_long valid", ++subnum);
    {
        const char *valid[] = {"0", "123", "-456",
                               "9223372036854775807", "-9223372036854775808"};
        long        expected[] = {0L, 123L, -456L, 9223372036854775807L, LONG_MIN};
        bool        ok;
        for (int i = 0; i < COUNT(valid); i++) {
            long    result;
            test_validate(
                (ok = try_parse_long(valid[i], &result) ) && result == expected[i],
                "try_parse_long('%s'): expected %ld, got %ld (ok=%s)",
                valid[i], expected[i], result, bool_str(ok));
        }
    }

    /* 5. Неудачи для long */
    test_sub("subtest %d: try_parse_long invalid", ++subnum);
    {
        const char *invalid[] = {
            "", "abc", "123abc", /* "  123", */ "123  ",
            "99999999999999999999", "-99999999999999999999"
        };
        for (int i = 0; i < COUNT(invalid); i++) {
            long    result;
            bool    ok;
            test_validate(
                !(ok = try_parse_long(invalid[i], &result) ),
                "try_parse_long('%s'): must fail, got ok=%s, result=%ld",
                invalid[i], bool_str(ok), result);
        }
    }

    /* 6. NULL */
    test_sub("subtest %d: try_parse_long NULL", ++subnum);
    {
        long result;
        test_validate(
            !try_parse_long(NULL, &result),
            "try_parse_long(NULL) must return false");
    }

    /* ========== try_parse_double ========== */

    /* 7. Успешный парсинг double (обычные числа) */
    test_sub("subtest %d: try_parse_double valid", ++subnum);
    {
        struct { const char *s; double d; } tests[] = {
            {"0.0", 0.0}, {"3.1415", 3.1415}, {"-2.718", -2.718},
            {"1.0e10", 1.0e10}, {"-1.0e-10", -1.0e-10}
        };
        for (int i = 0; i < COUNT(tests); i++) {
            double      result;
            bool        ok;
            test_validate(
                (ok = try_parse_double(tests[i].s, &result) ) && fabs(result - tests[i].d) < 1e-12 * fabs(tests[i].d + 1.0),
                "try_parse_double('%s'): expected %g, got %g (ok=%s)",
                tests[i].s, tests[i].d, result, bool_str(ok));
        }
    }

    /* 8. Специальные значения double: inf, -inf, nan (если разрешены) */
    test_sub("subtest %d: try_parse_double special", ++subnum);
    {
        // inf и -inf обычно парсятся без ошибки, nan тоже, проверим
        const char *special[] = {"inf", "-inf", "nan"};
        for (int i = 0; i < COUNT(special); i++) {
            double      result;
            bool        ok = try_parse_double(special[i], &result);
            // Проверяем, что парсинг успешен и значение соответствует ожидаемому
            if (strcmp(special[i], "inf") == 0) {
                test_validate(ok && isinf(result) && result > 0,
                    "try_parse_double('inf'): expected +inf, got ok=%s, result=%g", bool_str(ok), result);
            } else if (strcmp(special[i], "-inf") == 0) {
                test_validate(ok && isinf(result) && result < 0,
                    "try_parse_double('-inf'): expected -inf, got ok=%s, result=%g", bool_str(ok), result);
            } else if (strcmp(special[i], "nan") == 0) {
                test_validate(ok && isnan(result),
                    "try_parse_double('nan'): expected NaN, got ok=%s, result=%g", bool_str(ok), result);
            }
        }
    }

    /* 9. Неудачи для double (пустая строка, буквы, пробелы, переполнение) */
    test_sub("subtest %d: try_parse_double invalid", ++subnum);
    {
        const char *invalid[] = {
            "", "abc", "123abc", /*"  123" ,*/ "123  ",
            "1e9999", "-1e9999"
        };
        for (int i = 0; i < COUNT(invalid); i++) {
            double      result;
            bool        ok;
            test_validate(
                !(ok = try_parse_double(invalid[i], &result) ),
                "try_parse_double('%s'): must fail, got ok=%s, result=%g",
                invalid[i], bool_str(ok), result);
        }
    }

    /* 10. NULL */
    test_sub("subtest %d: try_parse_double NULL", ++subnum);
    {
        double result;
        test_validate(
            !try_parse_double(NULL, &result),
            "try_parse_double(NULL) must return false");
    }
    /* 11. try_parse_uint – корректные значения */
    test_sub("subtest %d: try_parse_uint valid", ++subnum);
    {
        const char *valid[] = {"0", "123", "  456", "4294967295"};
        unsigned int expected[] = {0, 123, 456, UINT_MAX};
        for (int i = 0; i < COUNT(valid); i++) {
            unsigned int res;
            test_validate(
                try_parse_uint(valid[i], &res) && res == expected[i],
                "try_parse_uint('%s'): expected %u, got %u",
                valid[i], expected[i], res
            );
        }
    }

    /* 12. try_parse_uint – некорректные строки */
    test_sub("subtest %d: try_parse_uint invalid", ++subnum);
    {
        const char *invalid[] = {
            "", "abc", "123abc", "123 ", " 123 ",
            "-1", "-0", "   -123", "  -1",
            "4294967296", "9999999999"
        };
        for (int i = 0; i < COUNT(invalid); i++) {
            unsigned int res;
            test_validate(
                !try_parse_uint(invalid[i], &res),
                "try_parse_uint('%s'): must fail, got %u", invalid[i], res
            );
        }
    }

    /* 13. try_parse_uint – NULL */
    test_sub("subtest %d: try_parse_uint NULL", ++subnum);
    {
        unsigned int res;
        test_validate(
            !try_parse_uint(NULL, &res),
            "try_parse_uint(NULL) must return false"
        );
    }

    /* 14. try_parse_ulong – корректные значения */
    test_sub("subtest %d: try_parse_ulong valid", ++subnum);
    {
        const char *valid[] = {"0", "123", "  456", "18446744073709551615"};
        unsigned long expected[] = {0UL, 123UL, 456UL, ULONG_MAX};
        for (int i = 0; i < COUNT(valid); i++) {
            unsigned long res;
            test_validate(
                try_parse_ulong(valid[i], &res) && res == expected[i],
                "try_parse_ulong('%s'): expected %lu, got %lu", valid[i], expected[i], res
            );
        }
    }

    /* 15. try_parse_ulong – некорректные строки */
    test_sub("subtest %d: try_parse_ulong invalid", ++subnum);
    {
        const char *invalid[] = {
            "", "abc", "123abc", "123 ", " 123 ",
            "-1", "-0", "   -123", "  -1",
            "18446744073709551616", "99999999999999999999"
        };
        for (int i = 0; i < COUNT(invalid); i++) {
            unsigned long res;
            test_validate(
                !try_parse_ulong(invalid[i], &res),
                "try_parse_ulong('%s'): must fail, got %lu", invalid[i], res
            );
        }
    }

    /* 16. try_parse_ulong – NULL */
    test_sub("subtest %d: try_parse_ulong NULL", ++subnum);
    {
        unsigned long res;
        test_validate(
            !try_parse_ulong(NULL, &res),
            "try_parse_ulong(NULL) must return false"
        );
    }
    return logret(TEST_PASSED, "done");
}

// ------------------------- TEST int_(not)in ---------------------------------

static TestStatus
tf_int_in(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    /* 1. int_in: значение присутствует в начале */
    test_sub("subtest %d: int_in – found at start", ++subnum);
    test_validate(
        int_in(10, 10, 20, 30),
        "10 must be in {10,20,30}"
    );

    /* 2. int_in: значение присутствует в середине */
    test_sub("subtest %d: int_in – found in middle", ++subnum);
    test_validate(
        int_in(20, 10, 20, 30),
        "20 must be in {10,20,30}"
    );

    /* 3. int_in: значение присутствует в конце */
    test_sub("subtest %d: int_in – found at end", ++subnum);
    test_validate(
        int_in(30, 10, 20, 30),
        "30 must be in {10,20,30}"
    );

    /* 4. int_in: значение отсутствует */
    test_sub("subtest %d: int_in – not found", ++subnum);
    test_validate(
        !int_in(99, 10, 20, 30),
        "99 must NOT be in {10,20,30}"
    );

    /* 5. int_notin: значение отсутствует → true */
    test_sub("subtest %d: int_notin – not found", ++subnum);
    test_validate(
        int_notin(99, 10, 20, 30),
        "99 must be NOT in {10,20,30}"
    );

    /* 6. int_notin: значение присутствует → false */
    test_sub("subtest %d: int_notin – found", ++subnum);
    test_validate(
        !int_notin(10, 10, 20, 30),
        "10 must be in {10,20,30} (notin=false)"
    );

    /* 7. Пустой список: int_in должно вернуть false */
    test_sub("subtest %d: int_in with empty list", ++subnum);
    test_validate(
        !int_in(42),
        "42 must NOT be found in empty list"
    );

    /* 8. Пустой список: int_notin должно вернуть true */
    test_sub("subtest %d: int_notin with empty list", ++subnum);
    test_validate(
        int_notin(42),
        "42 must be NOT in empty list"
    );

    /* 9. Один элемент в списке, совпадает */
    test_sub("subtest %d: int_in – single element match", ++subnum);
    test_validate(
        int_in(7, 7),
        "7 must be in {7}"
    );

    /* 10. Один элемент в списке, не совпадает */
    test_sub("subtest %d: int_in – single element no match", ++subnum);
    test_validate(
        !int_in(8, 7),
        "8 must NOT be in {7}"
    );

    /* 11. Повторяющиеся значения в списке */
    test_sub("subtest %d: int_in with duplicates", ++subnum);
    test_validate(
        int_in(5, 5, 5, 5),
        "5 must be found in {5,5,5}"
    );

    /* 12. Граничные значения (INT_MAX, INT_MIN) */
    test_sub("subtest %d: int_in with INT_MAX", ++subnum);
    test_validate(
        int_in(INT_MAX, INT_MIN, 0, INT_MAX),
        "INT_MAX must be found"
    );

    /* 13. int_in с очень большим списком (проверка быстродействия) */
    test_sub("subtest %d: int_in – large list", ++subnum);
    // Максимально допустимый список – около 100 элементов, проверим, что макрос не падает
    test_validate(
        int_in(100,
            1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,
            21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,
            41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,
            61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,
            81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,96,97,98,99,100),
        "100 must be in the large list"
    );

    /* 14. int_notin с большим списком */
    test_sub("subtest %d: int_notin – large list", ++subnum);
    test_validate(
        int_notin(0,
            1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,
            21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,
            41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,
            61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,
            81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,96,97,98,99,100),
        "0 must NOT be in the large list"
    );

    return logret(TEST_PASSED, "done");
}

// ------------------------- TEST comparators ---------------------------------

static TestStatus
tf_comparators(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    /* ---------- compare_int ---------- */
    test_sub("subtest %d: compare_int equal", ++subnum);
    test_validate(compare_int(42, 42) == 0, "42 must equal 42");

    test_sub("subtest %d: compare_int less", ++subnum);
    test_validate(compare_int(10, 20) < 0, "10 must be less than 20");

    test_sub("subtest %d: compare_int greater", ++subnum);
    test_validate(compare_int(30, 20) > 0, "30 must be greater than 20");

    /* ---------- compare_long ---------- */
    test_sub("subtest %d: compare_long equal", ++subnum);
    test_validate(compare_long(999999999L, 999999999L) == 0, "999999999L must equal 999999999L");

    test_sub("subtest %d: compare_long less", ++subnum);
    test_validate(compare_long(-100L, 0L) < 0, "-100L must be less than 0L");

    test_sub("subtest %d: compare_long greater", ++subnum);
    test_validate(compare_long(0L, -100L) > 0, "0L must be greater than -100L");

    /* ---------- compare_dbl ---------- */
    test_sub("subtest %d: compare_dbl equal", ++subnum);
    test_validate(compare_dbl(3.1415, 3.1415) == 0, "3.1415 must equal 3.1415");

    test_sub("subtest %d: compare_dbl less", ++subnum);
    test_validate(compare_dbl(1.0, 2.0) < 0, "1.0 must be less than 2.0");

    test_sub("subtest %d: compare_dbl greater", ++subnum);
    test_validate(compare_dbl(2.0, 1.0) > 0, "2.0 must be greater than 1.0");

    test_sub("subtest %d: compare_dbl NaN equal", ++subnum);
    test_validate(compare_dbl(NAN, NAN) == 0, "NaN must equal NaN (by implementation)");

    test_sub("subtest %d: compare_dbl NaN less", ++subnum);
    test_validate(compare_dbl(NAN, 1.0) < 0, "NaN must be less than any number (by implementation)");

    test_sub("subtest %d: compare_dbl +inf greater", ++subnum);
    test_validate(compare_dbl(INFINITY, 1.0) > 0, "+inf must be greater than 1.0");

    test_sub("subtest %d: compare_dbl -inf less", ++subnum);
    test_validate(compare_dbl(-INFINITY, 1.0) < 0, "-inf must be less than 1.0");

    test_sub("subtest %d: compare_dbl +inf vs -inf", ++subnum);
    test_validate(compare_dbl(INFINITY, -INFINITY) > 0, "+inf must be greater than -inf");

    /* ---------- compare_ptr ---------- */
    test_sub("subtest %d: compare_ptr equal", ++subnum);
    {
        int x = 0;
        test_validate(compare_ptr(&x, &x) == 0, "same address must be equal");
    }

    test_sub("subtest %d: compare_ptr not equal", ++subnum);
    {
        int a = 1, b = 2;
        // Порядок адресов на стеке не определён, поэтому просто проверяем, что результат не 0
        test_validate(compare_ptr(&a, &b) != 0, "different addresses must not be equal");
    }

    /* ---------- compare_char ---------- */
    test_sub("subtest %d: compare_char equal", ++subnum);
    test_validate(compare_char('A', 'A') == 0, "'A' must equal 'A'");

    test_sub("subtest %d: compare_char less", ++subnum);
    test_validate(compare_char('A', 'B') < 0, "'A' must be less than 'B'");

    test_sub("subtest %d: compare_char greater", ++subnum);
    test_validate(compare_char('B', 'A') > 0, "'B' must be greater than 'A'");

    /* ---------- compare_uint ---------- */
    test_sub("subtest %d: compare_uint equal", ++subnum);
    test_validate(compare_uint(123u, 123u) == 0, "123u must equal 123u");

    test_sub("subtest %d: compare_uint less", ++subnum);
    test_validate(compare_uint(10u, 20u) < 0, "10u must be less than 20u");

    test_sub("subtest %d: compare_uint greater", ++subnum);
    test_validate(compare_uint(20u, 10u) > 0, "20u must be greater than 10u");

    /* ---------- compare_ulong ---------- */
    test_sub("subtest %d: compare_ulong equal", ++subnum);
    test_validate(compare_ulong(999999999UL, 999999999UL) == 0, "999999999UL must equal 999999999UL");

    test_sub("subtest %d: compare_ulong less", ++subnum);
    test_validate(compare_ulong(1UL, 100UL) < 0, "1UL must be less than 100UL");

    test_sub("subtest %d: compare_ulong greater", ++subnum);
    test_validate(compare_ulong(100UL, 1UL) > 0, "100UL must be greater than 1UL");

    return logret(TEST_PASSED, "done");
}

// -------------------------------------------------------------------
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
        printf("Num %d\n", num);
            testenginestd_run(num,
                testnew(.f2 = tf_try_parse,        .num =  1, .name = "Simple try_parse_<type> test"              , .desc="", .mandatory=true)
              , testnew(.f2 = tf_int_in,           .num =  2, .name = "Simple tf_int_(not)in test"                , .desc="", .mandatory=true)
              , testnew(.f2 = tf_comparators,      .num =  3, .name = "Simple compare_<type> test"                , .desc="", .mandatory=true)
            );
        if (runall)
            break;
    }
    return logret(0, "end...");  // as replace of logclose()
}

#endif /* COMMONTESTING */

