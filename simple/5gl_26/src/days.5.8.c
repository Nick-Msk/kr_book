#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "log.h"
#include "common.h"
#include "checker.h"

typedef struct Fulldate {
    int     year;
    int     month;
    int     day;
    int     day_of_year;
} Fulldate;

#define INITFULLDATE(...) (Fulldate) {.year = -1, .month = -1, .day = -1, .day_of_year = -1, __VA_ARGS__ }

static int                       fprint_fulldate(FILE *f, Fulldate dt);

static bool                      calc_day_of_year(Fulldate *dt);

static bool                      calc_month_day(Fulldate *st);

static int                       parse_line(const char *restrict argv[], Fulldate *restrict dt){
    logenter("...");
    int     found = 0;
    if (!dt)
        return found;
    while (*argv){
        logauto( (void *)argv);
        const char *str = *argv++;
        logauto(str);
        while (isspace(*str) )
            str++;
        if (*str == '-'){
            found++;
            char c = *++str;
            logauto(c);
            switch (c){
                case 'm':
                    dt->month = atoi(str + 1);
                break;
                case 'd':
                    dt->day = atoi(str + 1);
                break;
                case 'y':
                    dt->year = atoi(str + 1);
                break;
                case 'f':
                    dt->day_of_year = atoi(str + 1);
                break;
                default:
                    fprintf(stderr, "Unsupported param [%c]\n", c);
                break;
            }
        }
    }
    return logret(found, "%d", found);
}

const char *usage_str = "Usage: %s -y<year> -m<month> -d<day> -f<day_of_year>\n";

int                             main(int argc, const char *argv[]){
    static const char *logfilename = "log/"__FILE__".log";
    loginit(logfilename, false, 0, "Start");    // TODO: rework that to LOG("logdir") or LOGAPPEND("logdir") or LOGSWITCH("logdir")

    if (argc > 1){
        if (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--version") == 0){
            printf("%s KR days of year, task 5.8\n", __FILE__);
            printf(usage_str, *argv);
            return 0;
        }
    }
    if (!check_arg(3, usage_str, *argv) ){
        return 2;
    }

    Fulldate dt = INITFULLDATE();
    parse_line(argv + 1, &dt);    // ... , 'y', &dt.year, 'm', &dt.month, 'd', &dt.day, 'f', &dt.day_of_year);

    //fprint_fulldate(stdout, dt);

    if (dt.day_of_year == -1){
        if (!calc_day_of_year(&dt) ){
            fprintf(stderr, "Incorrect day/month/year!\n");
            fprint_fulldate(stderr, dt);
        } else
            printf("Day of year %d\n", dt.day_of_year);
    } if (dt.month == -1 || dt.year == -1){
        if (!calc_month_day(&dt) ){
            fprintf(stderr, "Incorrect day_of_year/year!\n");
            fprint_fulldate(stderr, dt);
        } else
            printf("Month %d Day %d\n", dt.month, dt.day);
    }
    logclose("...");
    return 0;
}

static char daytab[2][13] = {
    {-1, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
    {-1, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
};

static inline bool               isleap(int year){
    return year % 4 == 0 && year % 100 != 0 && year % 400 != 0;
}

static bool                      calc_day_of_year(Fulldate *dt){
    if (!dt)
        return false;
    if (!inv(dt->year > 0 && dt->month <= 12 && dt->month >= 0 && dt->day > 0 && dt->day <= 31, "Incorrect day/month/year") )
        return false;
    dt->day_of_year = dt->day;
    int     leap = isleap(dt->year);
    for (int i = 1; i < dt->month; i++)
        dt->day_of_year += daytab[leap][i];
    return true;
}

static bool                      calc_month_day(Fulldate *dt){
    if (!dt)
        return false;
    if (!inv(dt->year > 0 && dt->day_of_year > 0 && dt->day_of_year <= 366, "Incorrect day_of_year/year") )
        return false;

    int     leap = isleap(dt->year), i;
    dt->day = dt->day_of_year;
    for (i = 1; dt->day > daytab[leap][i]; i++)
        dt->day -= daytab[leap][i];
    dt->month = i;
    return true;
}

static int                       fprint_fulldate(FILE *f, Fulldate dt){
    int cnt = 0;
    cnt += fprintf(f, "year [%d], month [%d], day [%d], day_of_year [%d]\n", dt.year, dt.month, dt.day, dt.day_of_year);
    return cnt;
}

