#include <stdio.h>
#include <ctype.h>

// common.h  -  new version of init_check
#define             arg_check(argcmax, msg, ...)\
    ( argc < (argcmax) ? fprintf(stderr, msg, ##__VA_ARGS__), 0 : 1)

#define         MAXLINE     1000

int             main(int argc, const char *argv[]){
    double  sum = 0.0, atodbl(const char *);
    int     get_line(char *, int);
    char    buf[MAXLINE];

    while (get_line(buf, MAXLINE) > 0)
        printf("\t%g\n", sum += atodbl(buf));

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

int                 get_line(char *line, int max){
    int     i = 0;
    char    c = '\0';

    while (--max > 0 && (c = getchar()) != EOF && c != '\n')
        line[i++] = c;
    if (c == '\n')
        line[i++] = '\n';
    line[i] = '\0';
    return i;
}

