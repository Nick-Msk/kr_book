#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include "log.h"

static int          entab(int tabsize);

int                 main(int argc, const char *argv[]){

    static const char *logfilename = "log/1.21.task.log";
    loginit(logfilename, false, 0, "Start");    // TODO: rework that to LOG("logdir") or LOGAPPEND("logdir") or LOGSWITCH("logdir") 

    int     tabsize = 8;    // default
    if (argc > 1){
        if (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--version") == 0){
            printf("%s entab task 1.20 KR\nUsage: %s <tabsize=8>\n", __FILE__, *argv);
            return 0;
        }
        tabsize = atoi(argv[1]);
    }
                    
    int tabcnt = entab(tabsize);
    printf("%d tabs were created\n", tabcnt);
    logclose("...");
    return 0;
}

static inline int add_tab(int pos, int tabsize){
    return pos + (tabsize - pos % tabsize);
}

static int          entab(int tabsize){
    int c, tabcnt = 0;
    int spacepos = -1, pos = 0;
    while ( (c = getchar()) != EOF){
        if (c == ' '){
            if (spacepos == -1)
                spacepos = pos;
            pos++;
            //logsimple("In ' ': spacepos=%d, pos=%d", spacepos, pos);
        }
        else {
            // print all tabs
            if (spacepos != -1){
                logsimple("2: spacepos=%d, pos=%d, tavsize=%d", spacepos, pos, tabsize);
                if (spacepos + 1 == pos)
                    putchar(' '), spacepos++;
                else {
                    while (add_tab(spacepos, tabsize) <= pos){
                        putchar('\t'); spacepos = add_tab(spacepos, tabsize); tabcnt++;
                        //logsimple("2: new spacepos=%d", spacepos);
                    }
                    // and remaining ' '
                    logsimple("after 2: spacepos=%d, pos=%d", spacepos, pos);
                    while (spacepos < pos)
                        spacepos++, putchar(' ');
                }
            }
            spacepos = -1;  // reset
            if (c == '\n')
                pos = 0;
            else 
                pos++;
            putchar(c);
            //logsimple("3 end: pos=%d, c=[%c]", pos, c);
        }
    }
    return tabcnt;
}

