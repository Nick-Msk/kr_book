#include <stdio.h>
#include <ctype.h>
#include <strings.h>
#include "log.h"

typedef enum {
    SHOW_MODE_NUMBER    = 0,
    SHOW_MODE_HORIZONT  = 1,
    SHOW_MODE_VERTICAL  = 2
} SHOW_MODE;

static const        int MAX_LINE = 120;

static int              fill_words(int *arr, int max_sz, int *max_len, int *p_wc); 

static int              show_histogram(const int *arr, int sz, int maxcnt, SHOW_MODE mode);

int                     main(int argc, const char *argv[]){

    static const char *logfilename = "log/1.13.task.log";
    loginit(logfilename, false, 0, "Start");

    //LOG(const char *logfilename = "log/1.13.task.log");

    SHOW_MODE mode = SHOW_MODE_NUMBER;

    if (argc > 1 && strcmp(argv[1], "hist") == 0)
        mode = SHOW_MODE_HORIZONT;

    static const int MAX_LEN = 1000;
    int hist[MAX_LEN];
    int wc, max_len;

    int max_cnt = fill_words(hist, MAX_LEN, &max_len, &wc);
    printf("Total %d words, max cnt = %d amx len = %d\n", wc, max_cnt, max_len);

    show_histogram(hist, max_len, max_cnt, mode);

    logclose("Total=%d", wc);
    return 0;
}

static int              fill_words(int *arr, int max_sz, int *p_max_len, int *p_wc){

#define PROCESS_CURR() \
    {if (curr < max_sz){\
        arr[curr]++;\
        if (max_cnt < arr[curr])\
            max_cnt = arr[curr];\
        if (max_len < curr)\
            max_len = curr;\
    } else\
       fprintf(stderr, "word have lenght %d more than allowed %d\n", curr, max_sz);\
    curr = 0; }

    logenter("Max word sz=%d", max_sz);
    int c, curr = 0, wc = 0;
    int max_cnt = 0, max_len = 0;

    for (int i = 0; i < max_sz; i++)
        arr[i] = 0;

    while ( (c = getchar()) != EOF){
        if (isalnum(c))
            curr++;
        else if (curr > 0){
            PROCESS_CURR();
            wc++;
        }
    }

    if (curr > 0){
        PROCESS_CURR();
        wc++;
    }

    if (p_wc)
        *p_wc = wc;
    if (p_max_len)
        *p_max_len = max_len;
    return logret(max_cnt, "total %d, maxlen %d maxcnt %d", wc, max_len, max_cnt);
}

static int              printline(char c, int val, int maxval, int maxpos){
    //logsimple("val = %d, maxval = %d, maxpos = %d", val, maxval, maxpos);
    int cnt = (long) maxpos * val / maxval;
    for (int i = 0; i < cnt; i++)
        putchar(c);
    putchar('\n');
    return logsimpleret(cnt, "for %d cnt is %d", val, cnt);
}

static int              show_histogram(const int *arr, int sz, int maxcnt, SHOW_MODE mode){
    logenter("maxlen = %d, mode=%d", maxcnt, mode);
    int res = 0;
    switch (mode){
        case SHOW_MODE_NUMBER:
            for (int i = 0; i <= sz; i++)
                if (arr[i] > 0)
                    printf("Len[%d] - %d\n", i, arr[i]);
        break;
        case SHOW_MODE_HORIZONT:
            for (int i = 0; i <= sz; i++)
                if (arr[i] > 0){
                    printf("%3d: ", i);
                    printline('#', arr[i], maxcnt, MAX_LINE);
                }
        break;
        case SHOW_MODE_VERTICAL:
        break;
        default:
            fprintf(stderr, "Unsupported mode %d", mode);
        break;
    }
    return logret(res, ".");
}
