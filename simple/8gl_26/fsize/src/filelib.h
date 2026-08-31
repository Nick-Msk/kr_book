#ifndef _FILELIB_H
#define _FILELIB_H

static const int            MAX_NAME = 1024;
static const int            MAX_PATH = 8192;
static const int            MAX_DIR  = 1024;

typedef struct {
    long    ino;
    //fs      name;
    char    name[MAX_NAME];
} TDirent;

typedef struct {
    int     fd;
    TDirent  d;
} TDIR;

struct direct {
    ino_t   d_ino;
    char    d_name[MAX_DIR];
};


#endif /* !_FILELIB_H */
