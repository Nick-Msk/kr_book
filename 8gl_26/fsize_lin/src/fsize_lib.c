#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>

#include "fsize.h"

static const int	MAX_PATH = 8192;

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
    int                cnt = 0;
    char               name[MAX_PATH];
    struct dirent     *dp;
    DIR               *dfd;

    if ( (dfd = opendir(dir)) == 0){
        fprintf(stderr, "%s: can't open %s:%s", __func__, dir, strerror(errno));
		return 0;
	}	
    while ( (dp = readdir(dfd) ) != 0){
        if ( (strcmp(dp->d_name, ".") == 0) ||
            (strcmp(dp->d_name, "..") == 0) )
        continue;
        if (strlen(dir) + strlen(dp->d_name) + 2 >= sizeof(name))
            fprintf(stderr, "%s: name %s %s too long", __func__, dir, dp->d_name);
        else {
        	snprintf(name, sizeof(name), "%s/%s", dir, dp->d_name);
        	cnt += func(name);
    	}
	}
    closedir(dfd);
    return cnt;;
}

