#include <stdio.h>
#include <string.h>

#include "log.h"
#include "common.h"
#include "error.h"

const char *usage_str = "Usage: %s <val1:TYPE1> <val2:TYPE2>\n";

static int              filecopy(FILE *to, FILE *from);

int                     main(int argc, const char *argv[]){
    logsimpleinit("Start");

    FILE    *fp;
    const char *prog = *argv;

    if (argc == 1)
        filecopy(stdout, stdin);
    else
        while (--argc > 0){
            if ( (fp = fopen(*++argv, "r") ) == 0)
                userraiseint(ERR_UNABLE_OPEN_FILE_READ, "%s: unable to open file %s\n", prog, *argv);
            else {
                filecopy(stdout, fp);
                fclose(fp);
            }
        }
    if (ferror(stdout))
        userraiseint(ERR_STREAM_ERROR, "%s: error while writing to stdout\n", prog);
    return logret(0, "end...");  // as replace of logclose()
}

static int              filecopy(FILE *to, FILE *from){
    int     c, cnt = 0;
    while ( (c = getc(from) ) != EOF)
        putc(c, to), cnt++;
    return cnt;
}


