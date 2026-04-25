#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>

#include "fsize.h"
#include "log.h"
#include "checker.h"
//#include "fs.h"

typedef int             (*tfcn)(const char *);

static int              dirwalk(const char *dir, tfcn func);

static TDIR            *opentdir(const char *dir);
static TDirent         *readtdir(TDIR *);
static void             closetdir(TDIR *);

extern int              fsize(const char *name){
    invraise(name != 0, "NUll input");
    struct stat stbuf;
    int         cnt = 0;

    if (stat(name, &stbuf) < 0)
        return userraise(0, ERR_CANT_GET_STAT, "%s: can't get stat %s", __func__, name);

    if ( (stbuf.st_mode & S_IFMT) == S_IFDIR)
        cnt += dirwalk(name, fsize);
    printf("%8lld %s\n", stbuf.st_size, name);
    return logsimpleret(++cnt, "Stat %d", cnt);
}

static int              dirwalk(const char *dir, tfcn func){
    //fs                 name = FS();
    int                cnt = 0;
    char               name[MAX_PATH];
    TDirent           *dp;
    TDIR              *dfd;

    if ( (dfd = opentdir(dir)) == 0)
        return sysraise(0, "%s: can't open %s", __func__, dir);
    while ( (dp = readtdir(dfd) ) != 0){
        if ( (strcmp(dp->name, ".") == 0) ||
            (strcmp(dp->name, "..") == 0) )
        continue;
        if (strlen(dir) + strlen(dp->name) + 2 >= sizeof(name))
            return userraise(0, ERR_TOO_LONG_LINE, "%s: name %s %s too long", __func__, dir, dp->name);
        else {
            snprintf(name, sizeof(name), "%s/%s", dir, dp->name);
            cnt += func(name);
        }
    }
    closetdir(dfd);
    return cnt;
}

