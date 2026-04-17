#include <stdio.h>
#include <stdlib.h>

int         main(int argc, const char *argv[]){
    const char msg[] = "1234567890";
    int cnt = 5;
    if (argc > 1)
        cnt = atoi(argv[1]);
    printf("%*s\n", cnt, msg);
    return 0;
}
