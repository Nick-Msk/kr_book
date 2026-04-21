#ifndef _FILES_P85_H
#define _FILES_P85_H

#include "bool.h"
#include <stdio.h>

static const int            M_EOF         = -1;
static const int            M_BUFSIZE     = 1024;
static const int            M_OPEN_MAX    = 20;

typedef struct _iobuf {
    int          cnt;
    char        *ptr;
    char        *base;
    unsigned     flags;
    int          fd;
} MFILE;

static inline int       mtech_fprint(FILE *restrict out, MFILE *restrict f){
    return fprintf(out, "MFILE: cnt %d, ptr %p, base %p, diff %lu, flags %u, fd %d\n",
        f->cnt, f->ptr, f->base, f->ptr - f->base, f->flags, f->fd);
}

static inline int       mtech_print(MFILE *f){
    return mtech_fprint(stdout, f);
}

extern                  MFILE _iob[M_OPEN_MAX];

extern MFILE            *fin;
extern MFILE            *fout;
extern MFILE            *ferr;

enum MFLAGS { MF_READ = 0x1, MF_WRITE = 0x2, MF_UNBUF = 0x4, MF_EOF = 0x8, MF_ERR = 0x10 };

extern int             _fillbuf(MFILE *fp);
extern int             _flushbuf(int c, MFILE *fp);

extern MFILE           *mopen(const char *restrict filename, const char *restrict mode);

extern bool             mclose(MFILE *fp);

extern int              mflush(MFILE *fp);

static inline int       mgetpos(MFILE *fp){
    return lseek(fp->fd, 0L, SEEK_CUR);
}

static inline bool      mfeof(const MFILE *p){
    return p->flags & MF_EOF;
}

static inline bool      mferror(const MFILE *p){
    return p->flags & MF_ERR;
}

static inline bool      mfileno(const MFILE *p){
    return p->fd;
}

static inline int       mgetc(MFILE *p){
    return --p->cnt >= 0 ? (unsigned char) *p->ptr++ : _fillbuf(p);
}

static inline int       mputc(int c, MFILE *p){
    return --p->cnt > 0 ? *p->ptr++ = c : _flushbuf(c, p);
}

static inline int       mgetchar(void){
    return mgetc(fin);
}

static inline int       mputchar(int c){
    return mputc(c, fout);
}

#endif /* !_FILES_P85_H */
