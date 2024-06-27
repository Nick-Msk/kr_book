#include <stdio.h>
#include <stdlib.h>
#include "log.h"

// engine
int         get_alg5_num(int val);
int         binstr_to_int(const char *str);
int         serialize_as_2(int val, char *buf, int sz);
int         calc_bits(const char *buf, int sz);


int         main(int argc, const char *argv[]){
    logenter("argc = %d", argc);
    int     treshold = 40;    // default

    if (argc > 1)
        treshold  = atoi(argv[1]);
    int     min_val = -1, i = 0;    // start from 0

    /* TEST
    min_val = get_alg5_num(6);  printf("6: %d\n", min_val);
    min_val = get_alg5_num(4);  printf("4: %d\n", min_val); */

    while ( (min_val = get_alg5_num(i)) <= treshold)
        printf("%d for %d\n", min_val, i++);
    printf("min found = %d for %d\n", min_val, i);

    return logret(0, "%d", min_val);
}

int         reverse_str(char *buf, int sz){
    logenter("%s, %d", buf, sz);
    for (int i = 0; i < sz / 2; i++){
        char c = buf[i];
        buf[i] = buf[sz - i - 1];
        buf[sz - i - 1]  = c;
    }
    return logret(sz, "reversed %s", buf);
}

// assume sz > 1
int         serialize_as_2(int val, char *buf, int sz){
    logenter("val = %d", val);
    int     i = 0;
    do {
        buf[i++] = (val & 1) + '0';
        val /= 2; //>>= 1;
    } while (i < sz - 1 && val > 0);
    buf[i] = '\0';
    logsimple("buf[%s], i %d", buf, i);
    reverse_str(buf, i);
    return logret(i, "ret %d", i);
}

int         calc_bits(const char *buf, int sz){
    int     cnt = 0;
    while (sz >= 0)
        if (buf[--sz] == '1')
            cnt++;
    return logsimpleret(cnt, "cnt = %d", cnt);
}

// calc remap to int -> int
int         get_alg5_num(int val){
    logenter("val = %d", val);
    char        buf[1000];  // mor ethat enough
    int         len = serialize_as_2(val, buf, sizeof buf);
    int         res = 0;    //

    if ((calc_bits(buf, len) & 1) == 1){       // odd 
        buf[len++] = '1';
        buf[len++] = '\0';
        logsimple("buf[%s]", buf);
        buf[0] = '1';
        buf[1] = '1';
    } else { // non odd
        buf[len++] = '0';
        buf[len++] = '\0';
        buf[0] = '1';
        buf[1] = '0';
    }
    res = binstr_to_int(buf);
    return logret(res, "%d", res);
}

// no OF checking!
int         binstr_to_int(const char *str){
    logenter("%s", str);
    int         res = 0;
    while (*str != '\0'){
        res = 2 * res + (*str - '0');
        str++;
    }
    return logret(res, "res = %d", res);
}
