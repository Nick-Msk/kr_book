#include <stdio.h>
#include <stdlib.h>

#include "guard.h"
#include "log.h"
#include "fs.h"
#include "common.h"

static int       kapr_next(fs *s);

int             main(int argc, const char *argv[]){
    logsimpleinit("Start");

    if (!check_arg(2, "Usage: %s 4-gidit value\n", *argv) )
        return 1;

    if (strlen(argv[1]) > 4){
        fprintf(stderr, "Digit must have <= 4 values\n");
        return 2;
    }

    fs      s = FS();
    int     val = atoi(argv[1]), prev = 0; // TODO: fs_getint/long/double
    fssprintf(s, "%04d", val);
    fstechprint(s);

    while (val != prev && RGUARDK ){  // TODO: RGUARDK(!next.v || (fscmp(s, next) == 0) )
        prev = val;
        val = kapr_next(&s);
    }
    printf("Last: %s\n", s.v);

    fsfree(s);

    if (!fs_alloc_check(false))
        logmsg("Warning: incorrect allocation of fs's");

    return logret(0, "Done");
}

static int       kapr_next(fs *s){
    fs_sort(s, false);     // sort as array! desc
    int     val1 = fs_getint(s);
    fs_reverse(*s);
    int     val2 = fs_getint(s);
    int     num = val1 - val2;  // val1 >= val2
    printf("%d - %d = next num = %d\n", val1, val2, num);
    fs_sprintf(s, "%4d", num);  // for the next iter
    return num;
}
