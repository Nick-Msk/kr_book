#include <fcntl.h>
#include <unistd.h>

#include "files.p8.5.h"
#include "checker.h"

static const int        PERM = 0666;

#define                 MFILE_INIT(...) (MFILE) {.fd = 0, .flags = 0, .base = 0, .ptr = 0, .cnt = 0, ##__VA_ARGS__}

// storage
MFILE                   _iob[M_OPEN_MAX] = {
    MFILE_INIT(.flags = MF_READ,                .fd = 0),
    MFILE_INIT(.flags = MF_WRITE,               .fd = 1),
    MFILE_INIT(.flags = MF_WRITE | MF_UNBUF,    .fd = 2)
};

MFILE           *fin      = _iob + 0;
MFILE           *fout     = _iob + 1;
MFILE           *ferr     = _iob + 2;

MFILE           *mopen(const char *restrict filename, const char *restrict mode){
    invraise(filename != 0 && mode != 0, "Null pointers");

    logenter("%s: %s", filename, mode);
    int      fd;
    MFILE   *fp = 0;

    if (*mode != 'r' && *mode != 'w' && *mode != 'a')
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

bool             munbuf(MFILE *fp){
    invraise(fp != 0, "Nullable mfile pointer");

    if (fp->base || ! (fp->flags & MF_WRITE) )
        return logsimpleerr(false, "File already used or not opened for write (%d)", fp->flags);
    fp->flags |= MF_UNBUF;
    return true;
}

bool             mclose(MFILE *fp){
    invraise(fp != 0, "Nullable mfile pointer");

    if ( (fp->flags & (MF_READ | MF_WRITE | MF_ERR) ) == MF_WRITE )
        mflush(fp);
    bool ret = close(fp->fd) == 0;
    free(fp->base);
    fp->base = fp->ptr = 0;
    fp->cnt = fp->flags = fp->fd = 0;
    return logsimpleret(ret, "%s", ret ? "Norm" : "Error while closing");
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

    if (--fp->cnt < 0){
        if (fp->cnt == -1)
            fp->flags |= MF_EOF;
        else
            fp->flags |= MF_ERR;
        fp->cnt = 0;
        return logsimpleret(M_EOF, "EOF or error");
    }
    return logsimpleret(*fp->ptr++, "Norm");
}

int             _flushbuf(int c, MFILE *fp){
    invraise(fp != 0, "Null pointer");

    if ( (fp->flags & (MF_READ | MF_WRITE | MF_ERR) ) != MF_WRITE)
        return logsimpleerr(M_EOF, "File is'nt open for write or in error state");

    int     bufsize = (fp->flags & MF_UNBUF) ? 1 : M_BUFSIZE;   // TODO: make bufsize a part of MFILE

    if (!fp->base){
        if ( (fp->base = malloc(bufsize) ) == 0)
            return logsimpleerr(M_EOF, "Unable to allocated %d", bufsize);
        fp->ptr = fp->base;
        fp->cnt = bufsize;
    }
    *fp->ptr++ = c;

    if (--fp->cnt <= 0){
        int     cnt = write(fp->fd, fp->base, bufsize);
        logsimple("write: %d, bufsize %d, cnt %d", cnt, bufsize, fp->cnt);
        fp->ptr = fp->base;
        fp->cnt = bufsize;
        if (cnt < bufsize){
            fp->flags |= MF_ERR;
            return logsimpleerr(M_EOF, "Error while writing");
        }
    }
    return logsimpleret(c, "Norm");
}

int              mflush(MFILE *fp){
    invraise(fp != 0, "Null pointer");

    if ( (fp->flags & (MF_READ | MF_WRITE | MF_ERR) ) != MF_WRITE )
        return logsimpleerr(M_EOF, "File is'nt open for write or in the error state (%d)", fp->flags);
    if ( fp->flags & MF_UNBUF)
        return logsimpleerr(M_EOF, "File is unbuffered");

    int     bytes = fp->ptr - fp->base;
    int     cnt = write(fp->fd, fp->base, bytes);

    if (cnt < bytes){
        fp->flags |= MF_ERR;
        return logsimpleerr(M_EOF, "Error while writing");
    }
    fp->ptr = fp->base;
    fp->cnt = M_BUFSIZE;
    return logsimpleret(0, "Flushed %d bytes", bytes);
}


