#include <dirent.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "filelib.h"

TDIR            *opentdir(const char *dir){
    int         fd;
    struct stat stbuf;
    TDIR       *dp;
    if ((fd = open(dir, O_RDONLY | O_DIRECTORY)) < 0
        || fstat(fd, &stbuf) < 0
        || (stbuf.st_mode & S_IFMT ) != S_IFDIR
        || (dp = malloc(sizeof(TDIR) ) ) == 0
    ){
	fprintf(stderr, "Unable to open dir %s", dir);
        return (TDIR *) 0;
	 }
    dp->fd = fd;
    return dp;
}

void             closetdir(TDIR *dp){
    if (dp){
        close(dp->fd);
        free(dp);
    }
}

TDirent         *readtdir(TDIR *dp){
    struct linux_dirent     dirbuf;
    static TDirent          d;

    // while read(dp->fd, (char *) &dirbuf,  sizeof(dirbuf) ) > 0 /* == sizeof(dirbuf) */ ){
    while (syscall(SYS_getdents, dp->fd, &dirbuf, dirbuf.d_reclen) != -1){
            //getdirentries(dp->fd, (char *) &dirbuf,  sizeof(dirbuf), 0) > 0) {
        if (dirbuf.d_ino == 0)
            continue;
        d.ino = dirbuf.d_ino;
        strncpy(d.name,  dirbuf.d_name, dirbuf.d_reclen );
        d.name[MAX_DIR - 1] = '\0';
        return &d;
    }
    return (TDirent *) 0;
}






