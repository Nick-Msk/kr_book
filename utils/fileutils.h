#ifndef _FILEUTILS_H
#define _FILEUTILS_H

#include <stdlib.h>
#include <string.h>

#include "fs.h"

/***************************************************************
                USEFUL MACRO AND FUNCTIONS
***************************************************************/

// ---------------------- API ----------------------------------------

// --------------- fs based API ---------------------------
extern int                      fgetline_fs(FILE *restrict in, fs *restrict s);

static inline int               getline_fs(fs *str){
    return fgetline_fs(stdin, str);
}

extern fs                       readfs_file(FILE *f);

// read all lines separately into fs[]
extern int                      freadlines(FILE *restrict f, fs **restrict lines);

static inline int               readlines(fs **lines){
    return freadlines(stdin, lines);
}

// OLD char * API
extern int                      get_line(char *line, int lim);
// write cnt fs into stream
extern int                      fwritelines(FILE *restrict f, const fs *restrict ptr, int cnt);

static inline int               writelines(const fs *lines, int cnt){
    return fwritelines(stdout, lines, cnt);
}

extern void                     freelines(fs *lineptr, int nlines);

// simpe file reader, must call free(s); after usage!!
extern char                     *read_from_file(FILE *f, int *p_cnt);


#endif /* ! _FILEUTILS_H */
