#ifndef _FILEUTILS_H
#define _FILEUTILS_H

#include <stdlib.h>
#include <string.h>

#include "fs.h"

/***************************************************************
                USEFUL MACRO AND FUNCTIONS
***************************************************************/

// ---------------------- API ----------------------------------------

// for now in common.c, then will be moved out
extern int                      get_line(char *line, int lim);

// for now in common.c, then will be moved out
extern int                      fgetline_fs(FILE *restrict in, fs *restrict s);

static inline int               getline_fs(fs *str){
    return fgetline_fs(stdin, str);
}

// simpe file reader, must call free(s); after usage!!
extern char                     *read_from_file(FILE *f, int *p_cnt);

#endif /* ! _FILEUTILS_H */
