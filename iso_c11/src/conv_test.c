#include <stdint.h>
#include <stdio.h>
#include <inttypes.h>

int     main(void){
    int i = -1;
    uint64_t v1 = (uint64_t) i;
    uint64_t v2 = (uint64_t)(int64_t) i;
    printf("v1 = %" PRIu64", v2 = %" PRIu64 "\n", v1, v2);
}
