#include <stdio.h>

// just test some printf formats
int         main(void){

    const char      msg[] = "Hello, World!";
    const char     *fmt[] = {":%s:", ":%10s:", ":%15s:", ":%.6s:", ":%.15s:", ":%-10s:", ":%-15s:", ":%15.10s:", ":%-15.10s:", 0};
    int             i = 0;
    while (fmt[i]){
        printf("\n\t\t\t\t%s, total %d\n", fmt[i], printf(fmt[i], msg) - 2);    // -2 because of two ::
        i++;
    }
/*
    printf("Total %d\n", printf(":%s", msg));
    printf("Total %d\n", printf(":%10s", msg));
    printf("Total %d\n", printf(":%15s", msg));
    printf("Total %d\n", printf(":%.6s", msg));
    printf("Total %d\n", printf(":%.15s", msg));
    printf("Total %d\n", printf(":%-10s", msg));
    printf("Total %d\n", printf(":%-15s", msg));
    printf("Total %d\n", printf(":%15.10s", msg));
    printf("Total %d\n", printf(":%-15.10s", msg)); */
    return 0;
}
