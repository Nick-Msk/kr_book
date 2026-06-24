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
            );
        if (runall)
            break;
    }
    return logret(0, "end...");  // as replace of logclose()
}

#endif /* COMMONTESTING */

