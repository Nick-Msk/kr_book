#include "buffer.h"
#include <string.h>
#include <stdio.h>
#include "log.h"

/****************************************** BUFFER ***************************************************/
#define             BUFSIZE     1000

static char         buf [BUFSIZE];
static int          bufp = 0;

int                 getch(void){
    logsimple("bufp = %d, [%c]",  bufp, (bufp > 0) ? buf[bufp - 1] : 'X');
    return (bufp > 0) ? buf[--bufp] : getchar();
}

int                 ungetch(int c){
    if (bufp >= BUFSIZE){
        fprintf(stderr, "Error: unable to ungetch symbol %c\n", c);
        return 0;
    }
    else {
        buf[bufp++] = c;
        return logsimpleret(1, "buf = %d, c = [%c]", bufp, c);
    }
}

int                 ungets(const char *s){ 
    int     cnt = strlen(s);
    logenter("cnt = %d, [%s], current bufp = %d", cnt, s, bufp);
    if (bufp + cnt > BUFSIZE){
        cnt = BUFSIZE - bufp;   // maximum amount to copy
        logsimple("2: cnt = %d", cnt);
    }
    memcpy(buf + bufp, s, cnt);
    bufp += cnt;
    return logret(cnt, "bufp = %d, cnt = %d", bufp, cnt);
}

// ------------------------------- Testing ----------------------------------
#ifdef BUFFERTESTING

#include "test.h"

// --------------------------------- TEST 1 ---------------------------------

static TestStatus           tf1(void)
{
    logenter("%s: Bare tests", __func__);

    char        c;
    int         i;

    // SUB 1
    ungetch('a');
    // CHEKC SUB 1
    if ((c = getch()) != 'a')
        return logret(TEST_FAILED, "c = [%c], but must be 'a'", c);
    if (bufp != 0)
        return logret(TEST_FAILED, "bufp = [%d], but must be 0", bufp);
    logmsg("SUB1 PASSED");

    // SUB 2
    for (i = 0; i < 20; i++)
        ungetch('a' + i);
    // CHECK SUB  2
    for (i = 20 - 1; i >= 0; i--)
        if ((c = getch()) != ('a' + i))
            return logret(TEST_FAILED, "SUB2: c[%d] = [%c], but must be [%c]", i, c, 'a' + i);
    if (bufp != 0)
        return logret(TEST_FAILED, "SUB2: bufp = [%d], but must be 0", bufp);
    logmsg("SUB2 PASSED");

    // SUB 3
    const char      pattern[] = "Just"; // "Just simple pattern to check";
    ungets(pattern);
    // CHECK SUB 3
    if (memcmp(pattern, buf, sizeof pattern - 1) != 0)
        return logret(TEST_FAILED, "SUB3: pattern [%s] != [%s]", pattern, buf);
    logmsg("SUB3 PASSED (sizeof pattern = %ld)", sizeof pattern);

    // SUB 4
    char            str[sizeof pattern];
    for (i = sizeof pattern - 2; i >= 0; i--)
        str[i] = getch();
    str[sizeof pattern - 1] = '\0';

    // CHECK SUB 4
    if (strcmp(pattern, str) != 0)
        return logret(TEST_FAILED, "SUB4: pattern [%s] != [%s]", pattern, str);

    return logret(TEST_PASSED, "done, 4 subtests");
}

// ---------------------------------------------------------------------------
int                         main(int argc, char *argv[])
{
    LOG(const char *logfilename = "log/buffer_test.log");   // TODO: rework that! It should be simple

    if (argc > 1)
        logfilename = argv[1];

    loginit(logfilename, false,  0, "Starting");    // TODO: should be loginitsimple("Starting")

        testenginestd(
            testnew(.f2 = tf1, .num = 1, .name = "Base test", .desc = "4 subtests"                               , .mandatory=true)
        );

    logclose("end...");
    return 0;
}


#endif /* BUFFERTESTING */

