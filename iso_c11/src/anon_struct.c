typedef struct str1 {
    int x;
} str1;

typedef struct str2 {
    //union {
        struct str3 {
            int x;
        };
    //    str1    s2;
    //};
    int y;
} str2;

int     f(str2 v){
    return v.x + v.y;   // check!
}

int     main(void){
    str2 p = {.x = 10, .y = 7};
    int res = f(p);
    return 0;
}
