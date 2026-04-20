#ifndef _FILES_P85_H
#define _FILES_P85_H

static const int            MEOF         = -1;
static const int            BUFSIZE     = 1024;
static const int            OPEN_MAX    = 20;

typedef struct _iobuf {
    int          cnt;
    char        *ptr;
    char        *base;
    unsigned     flags;
    int          fd;
} MFILE;

extern MFILE _iob[OPEN_MAX];

extern fin      = _iob + 0;
extern fout     = _iob + 1;
extern ferr     = _iob + 2;

enum FLAGS { _READ = 0x1, _WRITE = 0x2, _UNBUF = 0x4, _EOF = 0x8, _ERR = 0x10 };

extern int             _fillbuf(MFILE *p);
extern int             _flushbuf(int n, MFILE *);

extern MFILE           *mopen(const char *restrict filename, const char *restrict mode);

static inline bool      mfeof(const MFILE *p){
    return p->flags & _EOF;
}

static inline bool      mferror(const MFILE *p){
    return p->flags & _ERR;
}

static inline bool      mfileno(const MFILE *p){
    return p->fd;
}

static inline int       mgetc(MFILE *p){
    --p->cnt >= 0 ? (unsigned char) *p->ptr++ : _fillbuf(p);
}

static inline int       mputc(int c, MFILE *p){
    return --p->cnt >= 0 ? *p->ptr++ = c : _flushbuf(c, p);
}

static inline int       mgetchar(void){
    return mgetc(fin);
}

static inline           mputchar(int c){
    return mputc(c, fout);
}

#endif /* !_FILES_P85_H */
