#ifndef _FSIZE_H
#define _FSIZE_H

#include <sys/stat.h>
#include <sys/types.h>

//#include "fs.h"

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

extern int              fsize(const char *name);

#endif /* !_FSIZE_H */
