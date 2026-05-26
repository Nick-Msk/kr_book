typedef struct str1 {
    int t;
} str1;

typedef struct str2 {
    //union {
        struct {
            int x;
        };
    str1    s2;
    //};
    int y;
} str2;

int     f(str2 v){
    return v.x + v.y;   // check!
}

#include <stdio.h>

int     main(void){
    str2 p = {.x = 10, .y = 7};
    int res = f(p);
    printf("res = %d\n", res);
    return 0;
}
