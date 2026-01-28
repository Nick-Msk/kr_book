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


// TODO: to beffer.c
int                     getch(void){
    return bufp > 0 ? buf[--bufp] : getchar();
}
void                    ungetch(int c){
    if (bufp >= BUFSIZE)
        fprintf(stderr, "Unable to ungetch [%c], because of overflow (%d)\n", c, bufp);
    else
        buf[bufp++] = c;
}

static LexicOper        try_command(char c){
    logenter("[%c]", c);
    char    buf[10] = {c, '\0'};      // small one
    int     i = 0;
    LexicOper  ret = EOF;
    // get a command
    while (i < (int) sizeof(buf) - 1 && isalnum(c = buf[++i] = getch() ) )
        ;
    buf[i] = '\0';
    logmsg("buf [%s], c [%c]", buf, c);

    // currently there are only 3 commands: SIN COS TAN
    if (strcmp(buf, "sin") == 0)
        ret = LEXIC_SIN;
    else if (strcmp(buf, "cos") == 0)
        ret = LEXIC_COS;
    else if (strcmp (buf, "tan") == 0)
        ret = LEXIC_TAN;;
    // not a commmand, feed back all of that
    if ((int) ret == EOF)
        while (i > 1){
            logmsg("TO UNGETCH: i=%d, buf[i-1] = [%c])", i, buf[i - 1]);
            ungetch(buf[--i]);
        }
    if (c != EOF)
        ungetch(c);
    return logret(ret, "[%c - %s]", ret, LexicOperName(ret));
}

// -------------------------- (Utility) printers -------------------

// --------------------------- API ---------------------------------

LexicOper               lexic_getop(char *s, int sz){
    logenter("%d", sz);
    if (!inv(sz >= 10, "Buffer too small (%p)\n", sz) )
        return EOF;

    int     i = 0, c;

    while ( (s[i] = c = getch() ) == ' ' || c == '\t')
        ;
    s[1] = '\0';
    if (!isdigit(c) && c != '.' && c != '-' && c != '+' && c != ':' && c != '='){
        char    c1 = c;
        if (isalnum(c1)){
            if ( (c1 = try_command(c1)) != EOF)
                return c1;
        }
        return logret(c, "oper1: [%c - %s]", c, LexicOperName(c)); // oper
    }
    if (c == '=' || c == ':'){  // Variable
        char c1 = c;
        while (i < sz - 1 && (isalpha(s[i++] = c = tolower(getch() ) ) || c == ':' || c == '?' ) )     // [i++] to override '=' or ':' in  s[0] 
            ;
        s[i - 1] = '\0';
        if (c != EOF)
            ungetch(c);
        return logret(c1 == ':' ? LEXIC_VAR : LEXIC_ASSIGNMENT, "Vartiable %c [%s]", c1, s);
    }
    if (c == '+'|| c == '-'){
        int prev = c;   // + -  or digit
        if (!isdigit(c = getch() ) && c != '.'){
            ungetch(c);
            return logret(prev, "oper2: [%c - %s] c = [%c]", prev, LexicOperName(prev), c); // oper + or -
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
    if (c != EOF){
        ungetch(c); logmsg("Ungetch [%c]", c);  // TODO: remove log
    }

    return logret(LEXIC_NUMBER, "NUmber: [%s]", s);
}

// TODO: to buffer.c
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

// ---------------------- Testing Utilities -------------------------

#define CHECK_OP(op)\
{\
    LexicOper c;\
    typeof(op) op1 = (op);\
    if ( (c = lexic_getop(buf, sizeof(buf) ) ) != (op1) )\
        return logerr(TEST_FAILED, "Should be [%c - %s] but not [%c - %s]", (op1), LexicOperName(op1), c, LexicOperName(c));\
}

#define CHECK_BUF(val){\
    const char *val1 = (val);\
    if (strcmp(buf, val1) != 0)\
        return logerr(TEST_FAILED, "Buf contains [%s] but should be [%s]", buf, val1);\
}

// ------------------------- TEST 1 ---------------------------------

static TestStatus
tf1(const char *name)
{
    logenter("%s", name);

    int     subnum = 0;  // TODO: check if in VIRT_BOOK maked better!
    char    buf[1000];
    char    str[] = "12345.6789";

    // subtest 1
    {
        test_sub("subtest %d", ++subnum);
        for (int i = sizeof(str) - 1; i >= 0; i--)
            ungetch(str[i]);

        CHECK_OP(LEXIC_NUMBER);
        CHECK_BUF(str);
    }
    // subtest 2
    {
        test_sub("subtest %d", ++subnum);
        lexic_clear();
        if (bufp != 0){
            lexic_fprint(logfile);
            return logerr(TEST_FAILED, "Buffer point must be 0 but not %d", bufp);
        }
    }
    // subtest 3
    {
        test_sub("subtest %d", ++subnum);
        LexicOper    op = '+';
        ungetch('\n');
        ungetch(op);
        CHECK_OP(op);
    }
    return logret(TEST_PASSED, "done"); // TEST_FAILED
}


// ------------------------- TEST 2 ---------------------------------

static TestStatus
tf2(const char *name)
{
    logenter("%s", name);

    int     subnum = 0;  // TODO: check if in VIRT_BOOK maked better!
    char    buf[1000];
    char    str[] = "1.5 6 * 17.7 +\n";

    // subtest 1
    {
        test_sub("subtest %d", ++subnum);
        for (int i = sizeof(str) - 1; i >= 0; i--)
            ungetch(str[i]);

        CHECK_OP(LEXIC_NUMBER);
        CHECK_BUF("1.5");

        CHECK_OP(LEXIC_NUMBER);
        CHECK_BUF("6");

        CHECK_OP(LEXIC_MUL);

        CHECK_OP(LEXIC_NUMBER);
        CHECK_BUF("17.7");

        CHECK_OP(LEXIC_PLUS);
    }
    return logret(TEST_PASSED, "done"); // TEST_FAILED
}

// ------------------------- TEST 3 ---------------------------------

static TestStatus
tf3(const char *name)
{
    logenter("%s", name);

    int     subnum = 0;  // TODO: check if in VIRT_BOOK maked better!
    char    buf[1000];
    char    str[] = "-12345.6789";

    // subtest 1
    {
        lexic_clear();
        test_sub("subtest %d", ++subnum);

        for (int i = sizeof(str) - 1; i >= 0; i--)  // TODO: refactor to ungetcharr()
            ungetch(str[i]);

        CHECK_OP(LEXIC_NUMBER);
        CHECK_BUF(str);
    }
    return logret(TEST_PASSED, "done"); // TEST_FAILED
}

// ------------------------- TEST 4 ---------------------------------

static TestStatus
tf4(const char *name)
{
    logenter("%s", name);

    int     subnum = 0;  // TODO: check if in VIRT_BOOK maked better!
    char    buf[1000];
    char    str[] = "1.5 -6 * -17.7 +\n";

    // subtest 1
    {
        lexic_clear();
        test_sub("subtest %d", ++subnum);
        for (int i = sizeof(str) - 1; i >= 0; i--)  // TODO: refactor to ungetcharr()
            ungetch(str[i]);

        CHECK_OP(LEXIC_NUMBER);
        CHECK_BUF("1.5");

        CHECK_OP(LEXIC_NUMBER);
        CHECK_BUF("-6");

        CHECK_OP(LEXIC_MUL);

        CHECK_OP(LEXIC_NUMBER);
        CHECK_BUF("-17.7");

        CHECK_OP(LEXIC_PLUS);
    }
    return logret(TEST_PASSED, "done"); // TEST_FAILED
}

// ------------------------- TEST 5 ---------------------------------

static TestStatus
tf5(const char *name)
{
    logenter("%s", name);

    char        buf[1000];
    char        str[] = "11 -12.6789 sin +";
    int         subnum = 0;  // TODO: check if in VIRT_BOOK maked better!

    // subtest 1
    {
        lexic_clear();
        test_sub("subtest %d", ++subnum);

        for (int i = sizeof(str) - 1; i >= 0; i--)
            ungetch(str[i]);

        CHECK_OP(LEXIC_NUMBER);
        CHECK_BUF("11");

        CHECK_OP(LEXIC_NUMBER);
        CHECK_BUF("-12.6789");

        CHECK_OP(LEXIC_SIN);

        CHECK_OP(LEXIC_PLUS);
    }
    return logret(TEST_PASSED, "done"); // TEST_FAILED
}

// ------------------------- TEST 6 ---------------------------------

static TestStatus
tf6(const char *name)
{
    logenter("%s", name);

    char        buf[1000];
    char        str[] = "11 :v -12.345 + =z *";
    int         subnum = 0;  // TODO: check if in VIRT_BOOK maked better!

    // subtest 1
    {
        lexic_clear();
        test_sub("subtest %d", ++subnum);

        for (int i = sizeof(str) - 1; i >= 0; i--)
            ungetch(str[i]);

        CHECK_OP(LEXIC_NUMBER);
        CHECK_BUF("11");

        CHECK_OP(LEXIC_VAR);
        CHECK_BUF("v");

        CHECK_OP(LEXIC_NUMBER);
        CHECK_BUF("-12.345");

        CHECK_OP(LEXIC_PLUS);

        CHECK_OP(LEXIC_ASSIGNMENT);
        CHECK_BUF("z");

        CHECK_OP(LEXIC_MUL);
    }
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
      , testnew(.f2 = tf5, .num = 5, .name = "Simple oper (sin/cos) (w/o getchar)"   , .desc = "", .mandatory=true)
      , testnew(.f2 = tf6, .num = 6, .name = "Variable test (w/o getchar)"   , .desc = "", .mandatory=true)
    );

    logclose("end...");
    return 0;
}


#endif /* GETOPTESTING */

