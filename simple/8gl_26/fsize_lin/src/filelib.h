#ifndef _FILELIB_H
#define _FILELIB_H

#define            MAX_NAME  1024
#define            MAX_PATH  8192
#define            MAX_DIR   1024

typedef struct {
    long    ino;
    //fs      name;
    char    name[MAX_NAME];
} TDirent;

typedef struct {
    int     fd;
    TDirent  d;
} TDIR;

struct linux_dirent {
               unsigned long  d_ino;     /* Inode number */
               unsigned long  d_off;     /* Offset to next linux_dirent */
               unsigned short d_reclen;  /* Length of this linux_dirent */
               char           d_name[];  /* Filename (null-terminated) */
                                 /* length is actually (d_reclen - 2 -
                                    offsetof(struct linux_dirent, d_name)) */
               /*
               char           pad;       // Zero padding byte
               char           d_type;    // File type (only since Linux
                                         // 2.6.4); offset is (d_reclen - 1)
               */
           };

extern TDIR            *opentdir(const char *dir);
extern void             closetdir(TDIR *dp);
extern TDirent         *readtdir(TDIR *dp);

#endif /* !_FILELIB_H */
