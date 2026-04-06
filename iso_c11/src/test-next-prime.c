#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>

static int          calc_next_prime(int num);

int                 main(int argc, const char *argv[]){
    if (argc < 2){
        fprintf(stderr, "Usage: %s num\n", *argv);
        return 1;
    }
    int     num = atoi(argv[1]);
    if (num <= 2){
        fprintf(stderr, "Must be > 2\n");
        return 2;
    }
    printf("Next prime = %d\n", calc_next_prime(num) );
    return 0;
}

static              int isprime(int n) // assuming n > 1
{
    int i, root;
    if (n % 2 == 0 || n % 3 == 0)
        return false;
    root = (int) sqrt(n + 0.0);
    for (i = 5; i <= root; i += 6)
    {
        if (n % i == 0)
           return false;
    }
    for (i = 7; i <= root; i += 6)
    {
        if (n % i == 0)
           return 0;
    }
    return 1;
}

static int          calc_next_prime(int num){
    while (!isprime(num) )
        num++;
    return num;
}
