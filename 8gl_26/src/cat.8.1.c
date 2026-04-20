#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

#include "log.h"
#include "common.h"
#include "error.h"

const char                 *usage_str = "Usage: %s <val1:TYPE1> <val2:TYPE2>\n";
static const int            in        = 0;
static const int            out       = 1;
static const int            BUFSIZE   = 1024;

// via syscalls
static long                 filecopy(int to, int from);

int                         main(int argc, const char *argv[]){
    logsimpleinit("Start");

    int         fp;
    const char *prog = *argv;

    if (argc == 1)
        filecopy(out, in);
    else
        while (--argc > 0){
            if ( (fp = open(*++argv, O_RDONLY) ) < 0)
                userraiseint(ERR_UNABLE_OPEN_FILE_READ, "%s: unable to open file %s\n", prog, *argv);
            else {
                filecopy(out, fp);
                close(fp);
            }
        }
    //if (ferror(stdout))
      //  userraiseint(ERR_STREAM_ERROR, "%s: error while writing to stdout\n", prog);
    return logret(0, "end...");  // as replace of logclose()
}

static int          lgetchar(int from){
    static char    buf[BUFSIZE];
    static char   *bufp = buf;
    static int     n = 0;

    if (n <= 0){
        if ( (n = read(from, buf, BUFSIZE) ) < 0)
            userraiseint(ERR_STREAM_ERROR, "Error when read()");
        bufp = buf;
    }
    //logsimple("from %d, n == %d, bufp = %lu", from, n, bufp - buf);
    return (--n >= 0) ? (unsigned char) *bufp++ : EOF;
}

static int          lwritechar(int c, int to){
    static char    buf[BUFSIZE];
    static char   *bufp = buf;
    static int     n = 0;

    if (n >= BUFSIZE || c == EOF){   // buffer is full or EOF (fflush)
        int tmp;
        if ( (tmp = write(to, buf, n) ) < n)
            userraiseint(ERR_STREAM_ERROR, "Error when write()");
        bufp = buf;
        n = 0;
    }
    // n < BUFSIZE here!
    if (c != EOF){
       *bufp++ = (unsigned char) c;
        n++;
    }
    return c;
}


static long              filecopy(int to, int from){
    int     c;
    long    cnt = 0;
    while ( (c = lgetchar(from) ) != EOF)
        lwritechar(c, to), cnt++;
    lwritechar(EOF, to);
    return cnt;
}


