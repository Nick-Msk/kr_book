#include <stdio.h>
#include <ctype.h>
#include "getop.h"
#include "bool.h"
#include "log.h"
#include "checker.h"

/********************************************************************
                 STACK MODULE IMPLEMENTATION
********************************************************************/

// static globals

static const int        BUFSIZE = 100;
static char             buf[BUFSIZE];
static int              bufp = 0;

// internal type

// ---------- pseudo-header for utility procedures -----------------

// ------------------------- CONSTRUCTOTS/DESTRUCTORS --------------


// ------------------------------ Utilities ------------------------

int                     getch(void){
    return bufp > 0 ? buf[--bufp] : getchar();
}
void                    ungetch(int c){
    if (bufp >= BUFSIZE)
        fprintf(stderr, "Unable to ungetch [%c], because of overflow (%d)\n", c, bufp);
    else
        buf[bufp++] = c;
}

// -------------------------- (Utility) printers -------------------

// --------------------------- API ---------------------------------

int                     lexic_getop(char *s, int sz){
    logenter("%d", sz);
    if (!inv(sz >= 10, "Buffer too small (%p)\n", sz) )
        return EOF;

    int     i = 0, c;

    while ( (s[i] = c = getch() ) == ' ' || c == '\t')
        ;
    s[1] = '\0';
    if (!isdigit(c) && c != '.' && c != '-' && c != '+')
        return logret(c, "oper1: [%c]", c); // oper
    if (c == '+'|| c == '-'){
        int prev = c;   // + -  or digit
        if (!isdigit(c = getch() ) && c != '.'){
            ungetch(c);
            return logret(prev, "oper2: [%c] c = [%c]", prev, c); // oper + or -
        }
        s[++i] = c;
    }
    if (isdigit(c))
        while (i < sz - 1 && isdigit(s[++i] = c = getch() ) )
            ;
    if (c == '.')
        while (i < sz - 1 && isdigit(s[++i] = c = getch() ) )
            ;
    s[i] = '\0';
    if (c != EOF)
        ungetch(c), logmsg("Ungetch [%c]", c);

    return logret(LEXIC_NUMBER, "NUmber: [%s]", s);
}

void                    lexic_clear(void){
    bufp = 0;
}

// -------------------------- (API) printers -----------------------

int                     lexic_fprint(FILE *f){
    int cnt = 0;
    cnt += fprintf(f, "LEXIC: pos %d buffer[:", bufp);
    for (int i = 0; i < bufp; i++)
        fputc(buf[i], f), cnt++;
    cnt += fprintf(f, "]\n");
    return cnt;
}

// -------------------------------Testing --------------------------

#ifdef GETOPTESTING

#include <signal.h>
#include <strings.h>
#include "test.h"

// ------------------------- TEST 1 ---------------------------------

static TestStatus
tf1(void)
{
    logenter("Simple lexic test (w/o getchar, via buffer)");

    int     subnum = 0;  // TODO: check if in VIRT_BOOK maked better!
    char    buf[1000], c;
    char    dnum[] = "12345.6789";

    // subtest 1
    test_sub("subtest %d", ++subnum);
    for (int i = sizeof(dnum) - 1; i >= 0; i--)
        ungetch(dnum[i]);

    if ( (c = lexic_getop(buf, sizeof(buf) ) ) != LEXIC_NUMBER)
        return logerr(TEST_FAILED, "Shoud be numeric [%c] but not [%c]", LEXIC_NUMBER, c);
    if (strcmp(buf,  dnum) != 0)
        return logerr(TEST_FAILED, "getop returns [%s] but should be [%s]", buf, dnum);

    // subtest 2
    test_sub("subtest %d", ++subnum);
    lexic_clear();
    if (bufp != 0){
        lexic_fprint(logfile);
        return logerr(TEST_FAILED, "Buffer point must be 0 but not %d", bufp);
    }

    // subtest 3
    test_sub("subtest %d", ++subnum);
    char    op   = '+';
    ungetch('\n');
    ungetch(op);
    if ( (c = lexic_getop(buf, sizeof(buf) ) ) != op)
        return logerr(TEST_FAILED, "Shoud be oper [%c] but not [%c]", op, c);

    return logret(TEST_PASSED, "done"); // TEST_FAILED
}


// ------------------------- TEST 2 ---------------------------------

static TestStatus
tf2(void)
{
    logenter("Complex letic test (w/o getchar, via buffer)");

    int     subnum = 0;  // TODO: check if in VIRT_BOOK maked better!
    char    buf[1000], op, c;
    char    str[] = "1.5 6 * 17.7 +\n";

    // subtest 1
    test_sub("subtest %d", ++subnum);
    for (int i = sizeof(str) - 1; i >= 0; i--)
        ungetch(str[i]);

    op = LEXIC_NUMBER;
    if ( (c = lexic_getop(buf, sizeof(buf) ) ) != op)
        return logerr(TEST_FAILED, "Shoud be numeric [%c] but not [%c]", op, c);
    if (strcmp(buf, "1.5") != 0)
        return logerr(TEST_FAILED, "Getop returns [%s] but should be [%s]", buf, "1.5");
    op = LEXIC_NUMBER;
    if ( (c = lexic_getop(buf, sizeof(buf) ) ) != op)
        return logerr(TEST_FAILED, "Shoud be numeric [%c] but not [%c]", op, c);
    if (strcmp(buf, "6") != 0)
        return logerr(TEST_FAILED, "Getop returns [%s] but should be [%s]", buf, "6");
    op = '*';
    if ( (c = lexic_getop(buf, sizeof(buf) ) ) != op)
        return logerr(TEST_FAILED, "Shoud be numeric [%c] but not [%c]", op, c);
    op = LEXIC_NUMBER;
    if ( (c = lexic_getop(buf, sizeof(buf) ) ) != op)
        return logerr(TEST_FAILED, "Shoud be numeric [%c] but not [%c]", op, c);
    if (strcmp(buf, "17.7") != 0)
        return logerr(TEST_FAILED, "Getop returns [%s] but should be [%s]", buf, "17.7");
    op = '+';
    if ( (c = lexic_getop(buf, sizeof(buf) ) ) != op)
        return logerr(TEST_FAILED, "Shoud be numeric [%c] but not [%c]", op, c);

    return logret(TEST_PASSED, "done"); // TEST_FAILED
}

// ------------------------- TEST 3 ---------------------------------

static TestStatus
tf3(void)
{
    logenter("Negative numver test (w/o getchar)");

    char    buf[1000], c;
    char    dnum[] = "-12345.6789";

    lexic_clear();
    for (int i = sizeof(dnum) - 1; i >= 0; i--)
        ungetch(dnum[i]);

    if ( (c = lexic_getop(buf, sizeof(buf) ) ) != LEXIC_NUMBER)
        return logerr(TEST_FAILED, "Shoud be numeric [%c] but not [%c]", LEXIC_NUMBER, c);
    if (strcmp(buf,  dnum) != 0)
        return logerr(TEST_FAILED, "getop returns [%s] but should be [%s]", buf, dnum);

    return logret(TEST_PASSED, "done"); // TEST_FAILED
}

// ------------------------- TEST 4 ---------------------------------

static TestStatus
tf4(void)
{
    logenter("Complex letic test (w/o getchar, via buffer)");

    int     subnum = 0;  // TODO: check if in VIRT_BOOK maked better!
    char    buf[1000], op, c;
    char    str[] = "1.5 -6 * -17.7 +\n";

    // subtest 1
    lexic_clear();
    test_sub("subtest %d", ++subnum);
    for (int i = sizeof(str) - 1; i >= 0; i--)
        ungetch(str[i]);

    op = LEXIC_NUMBER;
    if ( (c = lexic_getop(buf, sizeof(buf) ) ) != op)
        return logerr(TEST_FAILED, "Shoud be numeric [%c] but not [%c]", op, c);
    if (strcmp(buf, "1.5") != 0)
        return logerr(TEST_FAILED, "Getop returns [%s] but should be [%s]", buf, "1.5");
    op = LEXIC_NUMBER;
    if ( (c = lexic_getop(buf, sizeof(buf) ) ) != op)
        return logerr(TEST_FAILED, "Shoud be numeric [%c] but not [%c]", op, c);
    if (strcmp(buf, "-6") != 0)
        return logerr(TEST_FAILED, "Getop returns [%s] but should be [%s]", buf, "-6");
    op = '*';
    if ( (c = lexic_getop(buf, sizeof(buf) ) ) != op)
        return logerr(TEST_FAILED, "Shoud be numeric [%c] but not [%c]", op, c);
    op = LEXIC_NUMBER;
    if ( (c = lexic_getop(buf, sizeof(buf) ) ) != op)
        return logerr(TEST_FAILED, "Shoud be numeric [%c] but not [%c]", op, c);
    if (strcmp(buf, "-17.7") != 0)
        return logerr(TEST_FAILED, "Getop returns [%s] but should be [%s]", buf, "-17.7");
    op = '+';
    if ( (c = lexic_getop(buf, sizeof(buf) ) ) != op)
        return logerr(TEST_FAILED, "Shoud be numeric [%c] but not [%c]", op, c);

    return logret(TEST_PASSED, "done"); // TEST_FAILED
}
// -------------------------------------------------------------------

int
main(int argc, char *argv[])
{
    const char *logfilename = "log/getop_test.log";

    if (argc > 1)
        logfilename = argv[1];

    loginit(logfilename, false, 0, "Starting");

    testenginestd(
        testnew(.f2 = tf1, .num = 1, .name = "Simple lexic test (w/o getchar)"       , .desc = "", .mandatory=true)
      , testnew(.f2 = tf2, .num = 2, .name = "Complex letic test (w/o getchar)"      , .desc = "", .mandatory=true)
      , testnew(.f2 = tf3, .num = 3, .name = "Negative numver test (w/o getchar)"    , .desc = "", .mandatory=true)
      , testnew(.f2 = tf4, .num = 4, .name = "Negative complex test (w/o getchar)"   , .desc = "", .mandatory=true)
    );

    logclose("end...");
    return 0;
}


#endif /* GETOPTESTING */

