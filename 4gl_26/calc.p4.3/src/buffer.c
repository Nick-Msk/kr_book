#include "stack.h"
#include "bool.h"

/********************************************************************
                 STACK MODULE IMPLEMENTATION
********************************************************************/

// static globals

static const int            BUFSIZE = 100;
static char                 buf[BUFSIZE];
static int                  bufp = 0;

// internal type

// ---------- pseudo-header for utility procedures -----------------

// ------------------------- CONSTRUCTOTS/DESTRUCTORS --------------

void                    buffer_clear(void){
    bufp = 0;
}

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

// -------------------------- (API) printers -----------------------

int                     buffer_fprint(FILE *f){
    int cnt = 0;
    cnt = fprintf(f, "BUFFER:\n");
    for (int i = 0; i < bufp; i++){
        cnt += fprintf(f, "[%d] - [%c]\t", i, buf[i]); 
        if ( (i + 1) % 10 == 0)
             cnt += fprintf(f, "\n");
    }
    cnt += fprintf(f, "\n-------------------------------------\n");
    return cnt;
}

// -------------------------------Testing --------------------------

#ifdef BUFFERTESTING

#include <signal.h>
#include "test.h"

//types for testing

// ------------------------- TEST 1 ---------------------------------

static TestStatus
tf1(const char *name)
{
    logenter("%s", name);
    int         subnum = 0;
    {
        test_sub("subtest %d", ++subnum);
        char res, some_val = 'e';
        ungetch(some_val);
        res = getch();
        if (res != some_val)
            return logerr(TEST_FAILED, "Getch returns [%c] but is must be [%c]", res, some_val);
    }
    {
        test_sub("subtest %d", ++subnum);

        char res, some_val;;
        for (int i = 0; i < 26; i++)
            ungetch(i + 'a');
        for (int i = 0; i < 26; i++){
            res = getch();
            some_val = 25 - i + 'a';
            if (res != some_val)
                return logerr(TEST_FAILED, "Getch returns [%c] but is must be [%c] (i = %d)", res, some_val, i);
        }
    }
    return logret(TEST_PASSED, "done"); // TEST_FAILED
}

// -------------------------------------------------------------------

int
main(int argc, char *argv[])
{
    const char *logfilename = "log/buffer.log";

    if (argc > 1)
        logfilename = argv[1];

    loginit(logfilename, false, 0, "Starting");

    testenginestd(
        testnew(.f2 = tf1, .num = 1, .name = "Simple and mult ungetch/getch test"       , .desc = "", .mandatory=true)
    );

    logclose("end...");
    return 0;
}


#endif /* BUFFERTESTING */

