// just to check max static array
#include <stdio.h>

#define     MAX_SIZE 1024 * 1024 * 1024L

unsigned long    arr[MAX_SIZE / sizeof(unsigned long)];

int     main(void){
    printf("Length = %lu %lu\n", sizeof(arr), sizeof(arr) / sizeof(unsigned long) );

    for (unsigned long l = 0; l < sizeof(arr) / sizeof(unsigned long); l++)
        arr[l] = l + 1;

    for (unsigned long l = 0; l < sizeof(arr) / sizeof(unsigned long); l++)
        if (arr[l] != l + 1){
            fprintf(stderr,  "Error on %lu\n", l);
            return 1;
        }
    printf("Passed\n");
    return 0;
}
