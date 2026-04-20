#include <fcntl.h>
#include <unistd.h>

#include "files.p8.5.h"
#include "checker.h"

static const int        PERM = 0666;

#define                 MFILE_INIT(...) (MFILE) {.fd = 0, .flags = 0, .base = 0, .ptr = 0, .cnt = 0, ##__VA_ARGS__}

// storage
MFILE                   _iob[M_OPEN_MAX] = {
    MFILE_INIT(.flags = MF_READ, .fd = 0),
    MFILE_INIT(.flags = MF_WRITE, .fd = 1),
    MFILE_INIT(.flags = MF_WRITE | MF_UNBUF, .fd = 2)
};

MFILE           *mopen(const char *restrict filename, const char *restrict mode){
    invraise(filename != 0 && mode != 0, "Null pointers");

    logenter("%s: %s", filename, mode);
    int      fd;
    MFILE   *fp = 0;

    if (*mode != 'r' || *mode != 'w' || *mode != 'a')
        return logerr( (MFILE *) 0, "Wrong format %s", mode);
    for (fp = _iob; fp < _iob + M_OPEN_MAX; fp++)
        if ( (fp->flags & (MF_READ | MF_WRITE) ) == 0)
            break;
    if (fp >= _iob + M_OPEN_MAX)
        return logerr( (MFILE *) 0, "Max limit exeded %d", M_OPEN_MAX);

    if (*mode == 'w')
        fd = creat(filename, PERM);
    else if (*mode == 'a') {
        if ( (fd = open(filename, O_WRONLY) ) < 0)
            fd = creat(filename, PERM);
        lseek(fd, 0L, SEEK_END);
    } else
       fd = open(filename, O_RDONLY);

    if (fd < 0)
         return logerr( (MFILE *) 0, "Unable to create %s", filename);
    fp->fd = fd;
    fp->cnt = 0;
    fp->base = 0;
    fp->flags = (*mode == 'r') ? MF_READ : MF_WRITE;

    return logret(fp, "%p", fp);
}

int             _fillbuf(MFILE *fp){
    invraise(fp != 0, "Nullable mfile pointer");
    if ( (fp->flags & (MF_READ | MF_WRITE | MF_ERR) ) != MF_READ)
        return logsimpleerr(M_EOF, "File is'nt open for read");

    int     bufsize = (fp->flags & MF_UNBUF) ? 1 : M_BUFSIZE;
    if (!fp->base)
        if ( (fp->base = malloc(bufsize) ) == 0)
            return logsimpleerr(M_EOF, "Unable to allocated %d", bufsize);

    fp->ptr = fp->base;
    fp->cnt = read(fp->fd, fp->ptr, bufsize);
    if (--fp->cnt > 0){
        if (fp->cnt == -1)
            fp->flags |= MF_EOF;
        else
            fp->flags |= MF_ERR;
        fp->cnt = 0;
        return logsimpleret(M_EOF, "EOF or error");
    }
    return logsimpleret(*fp->ptr++, "Norm");
}



