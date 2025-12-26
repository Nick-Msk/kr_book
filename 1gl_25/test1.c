#include <math.h>
#include <stdio.h>
#include <float.h>

int     main(void){
    double x = pow(0.0, 0.0);
    printf("Res1 = %g\n", x);
    x = pow(-2, 3);
    printf("Res2  = %g\n", x);
    x = pow(-2, 7.1);
    printf("Res3 = %g\n", x); 
    if(isnan(x))
        printf("Nan\n");
    return 0;
}

