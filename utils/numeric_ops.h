#ifndef _NUMERIC_OPS_H
#define _NUMERIC_OPS_H

#include <math.h>
#include <stdint.h>
#include "bool.h"

/***************************************************************
                    USEFUL NUMERIC MACRO AND FUNCTIONS
***************************************************************/


static inline int           isprime(int n) // assuming n > 1
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

static inline int           calc_next_prime(int num){
    while (!isprime(num) )
        num++;
    return num;
}

// adv API
extern unsigned             next_prime(unsigned n);

extern bool                 is_prime_miller(uint32_t n);

#endif /* !_NUMERIC_OPS_H */

