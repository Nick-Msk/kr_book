#include "common.h"
#include "log.h"
#include "fileutils.h"

/********************************************************************
                 FILEUTILS MODULE IMPLEMENTATION
********************************************************************/

// --------------------------- API ---------------------------------

// for now in common.c, then will be moved out
int                             get_line(char *line, int lim){
    int     c, i;
    for (i = 0; i < lim - 1 && (c = getchar()) != EOF && c != '\n'; i++)
        line[i] = c;
    if (c == '\n')
        line[i++] = c;
    line[i] = '\0';
    return i;
}

// for now in common.c, then will be moved out
extern fs                       fgetline_fs(FILE *in){
    fs      s = fsempty();
    int     c, i;
    for (i = 0; (c = fgetc(in)) != EOF && c != '\n'; i++)
        elem(s, i) = c;
    if (c == '\n')
        elem(s, i++) = c;
    elem(s, i) = '\0';
    return s;
}

char                           *read_from_file(FILE *f, int *p_cnt){
    logenter("read from input (%p)", f); 
    int      sz = 1024, len, pos = 0, cnt = sz; 
    char    *s = 0;        // string to store
    s = malloc(sz);
    if (!s){
        fprintf(stderr, "Unable to acclocate %d\n", sz);
        return 0;
    }   
    while ((len = fread(s + pos, 1, cnt - 1, f)) > 0){ 
        pos += len;
        logmsg("pos %d, len %d, sz %d", pos, len, sz);
        // check if next cnt bytes is available
        if (pos + cnt > sz - 1){
            s = realloc(s, sz *= 2);
            if (!s){
                fprintf(stderr, "Unable to acclocate %d\n", sz);
                free(s);
                return 0;
            }
            logmsg("new sz = %d", sz);
        }
    }
    s[pos] = '\0';      // to make a normal c-string
    if (p_cnt)
        *p_cnt = pos;
    return logret(s, "%d bytes were read", pos);
}

// -------------------------------Testing --------------------------

#ifdef FILEUTILSTESTING

//#include <signal.h>
#include "test.h"
#include "checker.h"

// ------------------------- TEST 1 ---------------------------------

static TestStatus
tf1(const char *name)
{
    logenter("%s", name);
    TFILE   tf;
    int         subnum = 0;
    {
        test_sub("subtest %d", ++subnum);
        
        char    pattern[] = "Test pattern ^&*()!@";
      //  const char fname[] = "common_1.test";       // file contains 3 lines
        TFILE test_fopen("test/");

        if (tfile(tf) == 0)
            return logret(TEST_SKIPPED. "Unable to create temporary file");

        fprintf("%s", pattern);
        

        fs s = getline_fs(f);
        
        return logactret(test_fclose(tf), TEST_FAILED, "Array is'nt freed");
    }

    test_fclose(tf);
    return logret(TEST_PASSED, "done"); // TEST_FAILED
}

// -------------------------------------------------------------------

int
main(int argc, char *argv[])
{
    const char *logfilename = "log/common.log";

    if (argc > 1)
        logfilename = argv[1];

    loginit(logfilename, false, 0, "Starting");

    testenginestd(
        testnew(.f2 = tf1, .num = 1, .name = "Getline_fs() simple test",                .desc = "", .mandatory=true)
    );

    logclose("end...");
    return 0;
}

#endif /* FILEUTILSTESTING */

