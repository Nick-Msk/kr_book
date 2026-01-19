#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

int         main(int argc, const char *argv[]){
    size_t      max_alloc = 64UL*1024*1024*1024;
    int         divider = 16;
    if (argc > 1)
        divider = atoi(argv[1]);
    if (argc > 2)
        max_alloc = atoi(argv[2]) * 1024 * 1024;

    printf("Try to alloc till %zuMb with divided = %d\n", max_alloc / 1024 / 1024, divider);
     
    void       *p = &divider;   // anything != 0
    size_t      alloc = max_alloc / divider;
    int         iter = 0;
    while (p != 0 && alloc <= max_alloc ){
        p = malloc(alloc);
        if (p)
            printf("%d: %zu Mb is allocated\n", iter++, alloc / 1024 / 1024);
        else
            printf("Unable to allocated %zu Mb - break\n", alloc / 1024 / 1024);
        free(p);
        alloc += max_alloc / divider;
    }

    return 0;
}

