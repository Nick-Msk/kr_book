#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#include "checker.h"

static unsigned             PERMS = 0666;

static long                 cp(int from, int to);

const char                 *usage_str = "Usage: %s from to\n";

int                        main(int argc, const char *argv[]){
    if ( !check_arg(3, usage_str, *argv) )
        return 3;

    int     from, to;
    if ( (from = open(argv[1], O_RDONLY, 0) ) < 0)
        return userraise(1, ERR_UNABLE_OPEN_FILE_READ, "Can't open %s for read\n", argv[1]);
    if ( (to = creat(argv[2], PERMS) ) < 0)
        return userraise(1, ERR_UNABLE_OPEN_FILE_WRITE, "Can't open %s for write, mode %04o\n", argv[2], PERMS);

    printf("Total %ld\n", cp(from, to) );
    close(from);
    close(to);

    return 0;
}

static long          cp(int from, int to){
    char    buf[BUFSIZ];
    long    total = 0;
    int     n;

    while ( (n = read(from, buf, BUFSIZ) ) > 0){
        total += n;
        if ( (write(to, buf, n) < n) )
            userraiseint(ERR_STREAM_ERROR, "Unable to write into %d", to);
    }
    return total;
}

