#include <stdint.h>
#include <stdio.h>


int     main(void){
    int i = -1;
    uint64_t v1 = (uint64_t) i;
    uint64_t v2 = (uint64_t)(int64_t) i;
    printf("v1 = %llu, v2 = %llu\n", v1, v2);
}
