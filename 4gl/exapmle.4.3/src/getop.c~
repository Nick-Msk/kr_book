#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#include "getop.h"
#include "log.h"
#include "buffer.h"

#define         MAXOP       100

char            op_buf[MAXOP];

/******************************************************************* TOKENS ********************************************************/
static bool         get_number(char *s, int sz);    // return true if number is parsed, otherwise ungetch all back and return false
static Oper         get_op(char *s, int sz);        // return operation or UNKNOWN
static void         skip_spaces(void);
static int          get_word_lowcase(char *s, int maxlen);

// probably is it possible to setup SOURCE instead of void
Oper                        getop(void){
    logenter("");

    Oper op = {EOF, "", 0.0, 0};    // TODO: possible to refactor normally?

    if (get_number(op_buf, sizeof op_buf)){
        op.dval = atof(op_buf);
        op.type = NUMBER;
        return logret(op, "number token %f", op.dval);
    }

    // otherwise return as operator or EOF
    op = get_op(op_buf, sizeof op_buf);

    return logret(op, "oper [%d]", op.type);
}

// + - * / % sin exp pow help  help quit printst
//  PLUS, MINUS, DIV, MUL, MOD, SIN, EXP, POW,
//  HELP, QUIT, PRINT_STACK, CLEAR, DOU, EXCH,
// GET, SET, UNSET, ARR
static Oper                 get_op(char *s, int sz){
    logenter("[%c]", s[0]);

    Oper res = {EOF, "", 0.0, 0};
    int     c = s[0] = tolower(getch());
    s[1] = '\0';     // sz definetly > 2

    switch (c){
        case EOF:
            return EOF; // as operation type
        break;
        case '+':
            c = PLUS;
        break;
        case '-':
            c = MINUS;
        break;
        case '*':
            c = MUL;
        break;
        case '/':
            c = DIV;
        break;
        case '%':
            c = MOD;
        break;
        default:    // commands or vars
            get_word_lowcase(s + 1, sz - 1);     // TODO: think how to optimize that!
            if (strcmp(s, "sin") == 0)
                c = SIN;
            else if (strcmp(s, "exp") == 0)
                c = EXP;
            else if (strcmp(s, "quit") == 0)
                c = QUIT;
            else if (strcmp(s, "help") == 0)
                c = HELP;
            else if (strcmp(s, "pow") == 0)
                c = POW;
            else if (strcmp(s, "pst") == 0)
                c = PRINT_STACK;
            else if (strcmp(s, "clear") == 0)
                c = CLEAR;
            else if (strcmp(s, "dou") == 0)
                c = DOU;
            else if (strcmp(s, "exch") == 0)
                c = EXCH;
            else if (strcmp(s, "get") == 0){
                c = GET;
                get_word_lowcase(s, sz);  // TODO: that is creepy, refactor that!
            }
            else if (strcmp(s, "set") == 0){
                c = SET;
                get_word_lowcase(s, sz);  // TODO: use Oper.str[] here!!!!
                logmsg("set : [%s]", s);
            }
            else if (strcmp(s, "unset") == 0){
                c = UNSET;
                get_word_lowcase(s, sz);
            }
            else if (strcmp(s, "arr") == 0){    //  arr v 4000
                c = ARR;
                get_word_lowcase(s, sz);
                logmsg("arr name = %s", s);
                strncpy(op.str, s, MAXNAME - 1);
                op.str[MAXNAME - 1] = '\0';
                if (!get_numbers(s, sz)){
                    fprintf(stderr, "Unable to init  ARR with [%s]\n", s);
                    c = UNKNOWN;
                }
                if ( (op.ival = atoi(s)) <= 0)
                    fprintf(stderr, "Unable to allocate array with %d elements", op.ival);
                    c = UNKNOWN;
                }
            }
            else
                c = UNKNOWN;
        break;
    }
    res.type = c;

    return logret(c, "oper [%d] [%s]", c, s);
}



static int          get_word_lowcase(char *s, int maxlen){
    skip_spaces();
    int     i = 0;
    while (i < maxlen - 1 && isalnum(c = getch()) )
        s[i++] = tolower(c);
    s[i] = '\0';
    return logsimpleret(i, "s = [%s]", s);
}



static bool         get_number(char *s, int sz){
    logenter("");

    skip_spaces();

    int c = s[0] = getch();
    int i = 0;

    if (c == '+' || c == '-')
        c = s[++i] = getch();

    if (!isdigit(c) && c != '.'){
        while (i >= 0)
            ungetch(s[i--]);
        return logret(false, "Not a number [%c]", c);
    }
    if (isdigit(c))
        while (i < sz - 1 && isdigit(s[++i] = c = getch()))
            ;
    if (c == '.')
        while (i < sz - 1 && isdigit(s[++i] = c = getch()))
            ;
    s[i] = '\0';
    if (c != EOF)
        ungetch(c);
    return logret(true, "number [%s]", s);
}

static void         skip_spaces(void){
    int     c;
    while ((c = getch()) == ' ' || c == '\t' || c == '\n') // '\n' is added!
        ;
    if (c != EOF){
        ungetch(c);
        logsimple("ungetch [%c]", c);
    }
}

// ------------------------------- Testing ----------------------------------
#ifdef GETOPTESTING

#include "test.h"

// --------------------------------- TEST 1 ---------------------------------

// Bare test
static TestStatus           tf1(void)
{
    logenter("%s: Bare test", __func__);

    const char testline1[] = " 54321"; // LOL
    logmsg("SUB1 load test line [%s]", testline1);
    ungets(testline1);
    Oper     op1;

    op1 = getop();
    logauto(op1.type);
    if (op1.type != NUMBER)
        return logret(TEST_FAILED, "SUB1 failed, must be NUMBER");
    if (op1.dval != 12345.0)
        return logret(TEST_FAILED, "SUB1 failed, dval = %f but must be %s", op1.dval, testline1);
    logmsg("SUB1 OK");  // TODO: think if pass subtests into test stats?

    const char testline2[] = " +";
    logmsg("SUB2 load test line [%s]", testline2);
    ungets(testline2);
    Oper    op2;

    op2 = getop();
    logauto(op2.type);
    if (op2.type != PLUS)
         return logret(TEST_FAILED, "SUB2 failed, must be PLUS");
    logmsg("SUB2 OK");

    const char testline3[] = " y tes"; // NOT GOOD TODO:
    logmsg("SUB3 load test line [%s]", testline3);
    ungets(testline3);
    Oper    op3;

    op3 = getop();
    logauto(op3.type);
    if (op3.type != SET)
        return logret(TEST_FAILED, "SUB3 failed, must be SET");
    if (strcmp(op3.str, "y") != 0)
        return logret(TEST_FAILED, "SUB3 failed, op3.str[%s] must be 'y'", op3.str);

    const char testline4[] = "pst ";
    for (int i = 0; i < (int) sizeof testline4 - 1; i++)
        ungetch(testline4[sizeof testline4 - 1 - i]);
    logmsg("SUB4 load test line [%s]", testline4);
    Oper    op4;

    op4 = getop();
    logauto(op4.type);
    if (op4.type != PRINT_STACK)
        return logret(TEST_FAILED, "SUB3 failed, op4 type = (%d) must be PRINT_STACK(%d)", op4.type, PRINT_STACK);
    logmsg("SUB4 OK");

    return logret(TEST_PASSED, "done 3 subtests"); // TEST_FAILED logmsg("SUB1 OK");
}

// ---------------------------------------------------------------------------
int                         main(int argc, char *argv[])
{
    LOG(const char *logfilename = "log/getop.log");   // TODO: rework that! It should be simple

    if (argc > 1)
        logfilename = argv[1];

    loginit(logfilename, false,  0, "Starting");    // TODO: should be loginitsimple("Starting")

        testenginestd(
            testnew(.f2 = tf1, .num = 1, .name = "Base test", .desc = "get operation"				, .mandatory=true)
        );

    logclose("End...");
    return 0;
}


#endif /* GETOPTESTING */



