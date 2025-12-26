#include <stdio.h>
#include <ctype.h>

// common.h  -  new version of init_check
#define             arg_check(argcmax, msg, ...)\
    ( argc < (argcmax) ? fprintf(stderr, msg, ##__VA_ARGS__), 0 : 1)

double              atodbl(const char *s);

int                 main(int argc, const char *argv[]){
    if (!arg_check(2, "Usage: %s double_val\n", *argv))
        return 1;
    double      val = atodbl(argv[1]);

    printf("Res = %g\n", val);
    return 0;
}

double              atodbl(const char *s){
    double  val, power;
    int     sign, i;

    for (i = 0; isspace(s[i]); i++)
        ;

    sign = s[i] == '-' ? -1 : 1;
    if (s[i] == '+' || s[i] == '-')
        i++;
    for (val = 0.0;  isdigit(s[i]); i++)
        val = val * 10.0 + (s[i] - '0');
    if (s[i] == '.')
        i++;
    for (power = 1.0; isdigit(s[i]); i++){
        val = val * 10.0 + (s[i] - '0');
        power *= 10.0;
    }

    return sign * val / power;
}

