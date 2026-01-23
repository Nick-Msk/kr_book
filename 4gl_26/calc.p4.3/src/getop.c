#include <stdio.h>
#include <ctype.h>
#include "getop.h"
#include "bool.h"

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
    if (sz < 2)
        return EOF;

    int     i = 0, c;

    while ( (s[0] = c = getch() ) == ' ' || c == '\t')
        ;
    s[1] = '\0';
    if (!isdigit(c) && c != '.')
        return c; // oper
    if (isdigit(c))
        while (i < sz - 1 && isdigit(s[++i] = c = getch() ) )
            ;
    if (c == '.')
        while (i < sz - 1 && isdigit(s[++i] = c = getch() ) )
            ;
    s[i] = '\0';
    if (c != EOF)
        ungetch(c);

    return LEXIC_NUMBER;
}

// -------------------------- (API) printers -----------------------

int                     lexic_fprint(FILE *f){
    int cnt = 0;
    fprintf(f, "LEXIC: \n");
    // noting for now
    return cnt;
}

// -------------------------------Testing --------------------------

#ifdef GETOPTESTING

#include "testing.h"
#include <signal.h>

// ------------------------- TEST 1 ---------------------------------

// ------------------------- TEST 2 ---------------------------------

// -------------------------------------------------------------------

// TODO:!!!!!!!!
int
main(int argc, char *argv[])
{
    const char *logfilename = "log/getop.log";

    if (argc > 1)
        logfilename = argv[1];

    loginit(logfilename, false, 0, "Starting");

    testenginestd(
        testnew(.f2 = tf1, .num = 1, .name = "Simple invariant text"       , .desc = "", .mandatory=true)
      , testnew(.f2 = tf2, .num = 2, .name = "Complex invariant test"      , .desc = "", .mandatory=true)
      //, testnew(.f2 = f3, .num = 3, .name = "Interrupt raising test"        , .desc = "Exception test."                                                             , .mandatory=true)
      //, testnew(.f2 = f4, .num = 4, .name = "System error test."            , .desc = "System error raising test (w/o exception)."  , .mandatory=true)
    );

        logclose("end...");
    return 0;
}


#endif /* GETOPTESTING */

