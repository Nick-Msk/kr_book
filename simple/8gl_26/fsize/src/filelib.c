#include "filelib.h"



static TDIR            *opentdir(const char *dir){
    int         fd;
    struct stat stbuf;
    TDIR       *dp;
    if ((fd = open(dir, O_RDONLY)) < 0
        || fstat(fd, &stbuf) < 0
        || (stbuf.st_mode & S_IFMT ) != S_IFDIR
        || (dp = malloc(sizeof(TDIR) ) ) == 0
    )
        return sysraise( (TDIR *) 0, "Unable to open dir %s", dir);
    dp->fd = fd;
    return logsimpleret(dp, "Dir [%s] is opened", dir);
}

static void             closetdir(TDIR *dp){
    logsimple("closing %s", dp->d.name);
    if (dp){
        close(dp->fd);
        free(dp);
    }
}

static TDirent         *readtdir(TDIR *dp){
    struct direct           dirbuf;
    static TDirent          d;

    logsimple("About to read [%d:%s]", dp->fd, dp->d.name);

    while ( //read(dp->fd, (char *) &dirbuf,  sizeof(dirbuf) ) > 0 /* == sizeof(dirbuf) */ ){
            getdirentries(dp->fd, (char *) &dirbuf,  sizeof(dirbuf), 0) > 0) {
        logauto(dirbuf.d_ino);
        if (dirbuf.d_ino == 0)
            continue;
        d.ino = dirbuf.d_ino;
        strncpy(d.name,  dirbuf.d_name, sizeof(dirbuf.d_name) );
        d.name[MAX_DIR - 1] = '\0';
        return logsimpleret(&d, "read next");
    }
    sysraise(0, "Read dir error");
    return logsimpleerr( (TDirent *) 0, "Unable to read");
}






