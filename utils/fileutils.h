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

// write cnt fs into stream
extern int                      fwritelines(FILE *restrict f, const fs *restrict ptr, int cnt);

static inline int               writelines(const fs *lines, int cnt){
    return fwritelines(stdout, lines, cnt);
}
// TODO: remove that after migration to fsarray
extern void                     freelines(fs *lineptr, int nlines);

extern int                      fprint_file(FILE *restrict out, FILE *restrict f);

static inline int               print_file(FILE *restrict f){
    return fprint_file(stdout, f);
}

// OLD char * API
extern int                      get_line(char *line, int lim);
// simpe file reader, must call free(s); after usage!!
extern char                     *read_from_file(FILE *f, int *p_cnt);

extern bool                     fread_pattern(FILE *restrict f, const char *restrict, int sz);
#define                         freadpattern(in, p) fread_pattern(in, (p), sizeof(p) - 1)


#endif /* ! _FILEUTILS_H */
