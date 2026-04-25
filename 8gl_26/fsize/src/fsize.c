#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>

#include "fsize.h"
#include "log.h"
#include "checker.h"
#include "fs.h"

typedef int             (*tfcn)(const fs *);

static int              dirwalk(const fs *dir, tfcn func);

extern int              fsize(const fs *name){
    invraise(name != 0, "NUll input");

    logsimple("Filename %s", name->v);
    struct stat stbuf;
    int         cnt = 0;

    if (stat(name->v, &stbuf) < 0)
        return userraise(0, ERR_CANT_GET_STAT, "%s: can't get stat %s", __func__, name->v);

    if ( (stbuf.st_mode & S_IFMT) == S_IFDIR)
        cnt += dirwalk(name, fsize);
    printf("%8lld %s\n", stbuf.st_size, name->v);
    return logsimpleret(++cnt, "Stat %d", cnt);
}

static int              dirwalk(const fs *dir, tfcn func){
    logsimple("dir %s", dir->v);
    fs                 name = FS();
    int                cnt = 0;
    //char               name[MAX_PATH];
    struct dirent     *dp;
    DIR               *dfd;

    if ( (dfd = opendir(dir->v)) == 0)
        return sysraise(0, "%s: can't open %s", __func__, dir->v);
    while ( (dp = readdir(dfd) ) != 0){
        if ( (strcmp(dp->d_name, ".") == 0) ||
            (strcmp(dp->d_name, "..") == 0) )
        continue;
        /*if (strlen(dir) + strlen(dp->d_name) + 2 >= sizeof(name))
            return userraise(0, ERR_TOO_LONG_LINE, "%s: name %s %s too long", __func__, dir, dp->d_name);
        else {*/
        fssprintf(name, "%s/%s", dir->v, dp->d_name);
        cnt += func(&name);
    }
    fsfree(name);
    closedir(dfd);
    return logsimpleret(cnt, "ret %d", cnt);
}

