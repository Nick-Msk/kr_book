#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>

#include "fsize.h"
#include "log.h"
#include "checker.h"
//#include "fs.h"

static const int        MAX_PATH = 8192;

typedef int             (*tfcn)(const char *);

static int              dirwalk(const char *dir, tfcn func);

extern int              fsize(const char *name){
    invraise(name != 0, "NUll input");

    logsimple("Filename %s", name);
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
    logsimple("dir %s", dir);
    //fs                 name = FS();
    int                cnt = 0;
    char               name[MAX_PATH];
    struct dirent     *dp;
    DIR               *dfd;

    if ( (dfd = opendir(dir)) == 0)
        return sysraise(0, "%s: can't open %s", __func__, dir);
    while ( (dp = readdir(dfd) ) != 0){
        if ( (strcmp(dp->d_name, ".") == 0) ||
            (strcmp(dp->d_name, "..") == 0) )
        continue;
        if (strlen(dir) + strlen(dp->d_name) + 2 >= sizeof(name))
            return userraise(0, ERR_TOO_LONG_LINE, "%s: name %s %s too long", __func__, dir, dp->d_name);
        else {
            snprintf(name, sizeof(name), "%s/%s", dir, dp->d_name);
            cnt += func(name);
        }
    }
    closedir(dfd);
    return cnt;
}

