#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>

#include "filelib.h"
#include "fsize.h"

//static const int	MAX_PATH = 8192;

typedef int             (*tfcn)(const char *);

static int              dirwalk(const char *dir, tfcn func);

extern int              fsize(const char *name){

    struct stat stbuf;
    int         cnt = 0;

    if (stat(name, &stbuf) < 0){
        fprintf(stderr, "%s: can't get stat %s: %s", __func__, name, strerror(errno) );
		return 0;
	}
    if ( (stbuf.st_mode & S_IFMT) == S_IFDIR)
        cnt += dirwalk(name, fsize);
    printf("%8ld %s\n", stbuf.st_size, name);
    return ++cnt;
}

static int              dirwalk(const char *dir, tfcn func){
    int                 cnt = 0;
    char                name[MAX_PATH];
    TDirent            *dp;
    TDIR               *dfd;

    if ( (dfd = opentdir(dir)) == 0){
        fprintf(stderr, "%s: can't open %s:%s", __func__, dir, strerror(errno));
		return 0;
	}	
    while ( (dp = readtdir(dfd) ) != 0){
        if ( (strcmp(dp->name, ".") == 0) ||
            (strcmp(dp->name, "..") == 0) )
        continue;
        if (strlen(dir) + strlen(dp->name) + 2 >= sizeof(name))
            fprintf(stderr, "%s: name %s %s too long", __func__, dir, dp->name);
        else {
        	snprintf(name, sizeof(name), "%s/%s", dir, dp->name);
        	cnt += func(name);
    	}
	}
    closetdir(dfd);
    return cnt;;
}

