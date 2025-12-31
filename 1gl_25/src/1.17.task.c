#include <stdio.h>
#include <stdlib.h>

static const int MAXLEN = 100;

static int      print_lines(int maxlen);

int             main(int argc, const char*argv[]){
    int maxlen = 80;    // default

    if (argc > 1)
        maxlen = atoi(argv[1]);

    printf("Total lines %d\n", print_lines(maxlen));
    return 0;
}

static int      print_lines(int maxlen){
    int len = 0, c, cnt = 0;
    char s[MAXLEN];
    while ( (c = getchar()) != EOF){
        if (c == '\n'){
            if (len > maxlen){
                s[len] = '\0';
                printf("%s\n", s);
                cnt++;
            }
            len = 0;
        } else if (len < MAXLEN) {
            s[len++] = c;
        } // else out of size
    }
    return cnt;
}
