#include <math.h>
#include <stdio.h>
#include <stdlib.h>

int         main(int argc, const char *argv[]){
    if (argc < 3){
        fprintf(stderr," Usage: %s <val:double> <val:double>\n", *argv);
        return 1;
    }
    double   x1 = atof(argv[1]), 
             x2 = atof(argv[2]);
    printf("%f ^ %f = %f\n", x1, x2, pow(x1, x2) );
    return 0;
}
