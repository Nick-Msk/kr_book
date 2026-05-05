#include <stdio.h>

int         main(void){
    int     val = 1;

    while (({ static int _guard_val = 10;  (_guard_val-- != 0) && (val > 0); }))
    {
        printf("val %d\n", val++);
    }
    /*while (({
        static int _guard_val = 10;
        (_guard_val-- != 0) && (val > 0);   // точку с запятой убрали
    }))*/ 
}
