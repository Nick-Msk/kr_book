#include <stdio.h>
#include <strings.h>
#include <stdlib.h>

#include "log.h"

static int              split_stream(int splitsize);

int                     main(int argc, const char *argv[]){
    static const char *logfilename = "log/1.25.task.log";
    loginit(logfilename, false, 0, "Start");    // TODO: rework that to LOG("logdir") or LOGAPPEND("logdir") or LOGSWITCH("logdir") 

    int     splitsize = 80;    // default
    if (argc > 1){
        if (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--version") == 0){
            printf("%s split long lines task 1.22 KR\nUsage: %s <splitsize=8>\n", __FILE__, *argv);
            return 0;
        }
        splitsize = atoi(argv[1]);
    }
    
    int splitcnt = split_stream(splitsize);
    
    printf("Total %d lines were created\n", splitcnt);
    logclose("...");
    return 0;
}

static int                split_stream(int splitsize){
    int     cnt = 0, pos = 0, spcpos = -1;
    int     c;
    //char    buf[splitsize + 1];

    while ( (c = getchar()) != EOF){
        if (c == '\n'){
            pos = 0;
            spcpos = -1;
            putchar(c);
        } else if (pos == splitsize){
            putchar('\n');
            cnt++;
            pos = 1;
            spcpos = -1;
            putchar(c); // on the new line
        } else if (c == ' ' || c == '\t'){ // TODO: actually tab isn't pretty equal to space...  probably tab shoud counted as + pos % tabsize
            if (spcpos == -1)
                spcpos = pos;   // fix first space occurence
            else
                spcpos++; 
            pos++;
        }
        else {
            if (spcpos > 0){
                printf("%*c", pos - spcpos - 1, ' ');
                spcpos = -1;
            }
            putchar(c);
            pos++;
        }
    }

    return cnt;
}

/*static int              split_stream(int splitsize){
    int cnt = 0, pos = 0, spcpos = -1;
    int c;
    char buf[splitsize + 1];

    while ( (c = getchar()) != EOF){
        logsimple("c=%c, pos=%d, spcpos=%d", c, pos, spcpos);
        if (c == ' ' || c == '\t'){
            if (spcpos == -1)
                spcpos = pos;
        }
        if (c == '\n' || pos == splitsize){
            logsimple("newline spcpos=%d, pos=%d", spcpos, pos);
            if (spcpos != -1)
                buf[spcpos] = '\0', spcpos = -1;
            else buf[pos] = '\0';

            printf("%s\n", buf);
            pos = 0;
        } else
            spcpos = -1; // find valuable data - reset
        buf[pos++] = c;
    }
    
    return cnt;
}*/
