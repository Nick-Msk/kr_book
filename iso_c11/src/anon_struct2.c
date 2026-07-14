#include <stdio.h>

// Динамический массив (владеет памятью)
typedef struct value64_tarray {
    int            *v;
    int            cnt;
    int            sz;
} value64_tarray;

// Статический массив (не владеет памятью)
typedef struct {
    struct value64_tarray base;   // общая часть, идентичная по layout
} value64_static_tarray;

int     main(void){
    value64_static_tarray tmp;
    value64_tarray orig;
    return 0;
}
