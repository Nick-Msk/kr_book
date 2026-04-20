#include <unistd.h>

static const int in     = 0;
static const int out    = 1;
static const int BUFSIZ = 1024;

int     main(void){

    char    buf[BUFSIZ];
    int     cnt;

    while( (cnt = read(in, buf, BUFSIZ) ) > 0)
        write(out, buf, cnt);

    return 0;
}

