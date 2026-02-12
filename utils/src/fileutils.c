
#include "log.h"
#include "common.h"
#include "fs.h"
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
extern int                      fgetline_fs(FILE *restrict in, fs *restrict s){
    int     c, i;
    for (i = 0; (c = fgetc(in)) != EOF && c != '\n'; i++){
        elem(*s, i) = c;
    }
    if (c == '\n')
        elem(*s, i++) = c;
    if (i > 0 || c != EOF)
        fsetlen(*s, i);    // fix length and set '\0'!
    return i;
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
    fs      s = fsempty();
    int         subnum = 0;
    {
        test_sub("subtest %d", ++subnum);

        char    pattern1[] = "Test pattern ^&*()!@\n";
        char    pattern2[] = "second";
      //  const char fname[] = "common_1.test";       // file contains 3 lines
        tf = test_fopen("/tmp/");

        if (tfile(tf) == 0)
            return logerr(TEST_SKIPPED, "Unable to create temporary file");

        if (fprintf(tfile(tf), "%s", pattern1) < 0 || fprintf(tfile(tf), "%s", pattern2) < 0){
            perror("Unable to printf...");
            return logacterr( test_fclose(tf), TEST_FAILED, "Unable to printf...");
        }
        fflush(tfile(tf));
        test_freset(tf);

        fgetline_fs(tfile(tf), &s);
        if (s.len + 1 != sizeof(pattern1))
            return logacterr( (fsfree(s), test_fclose(tf) ), TEST_FAILED, "Fs Len should be %lu but not %d", sizeof(pattern1) - 1, s.len);

        if (strcmp(pattern1, s.v) != 0)
            return logacterr( (fsfree(s), test_fclose(tf) ), TEST_FAILED, "Must be [%s] but not [%s]", pattern1, s.v);

        fgetline_fs(tfile(tf), &s);
        if (s.len + 1 != sizeof(pattern2))
            return logacterr( (fsfree(s), test_fclose(tf) ), TEST_FAILED, "Fs Len should be %lu but not %d", sizeof(pattern2) - 1, s.len);

        if (strcmp(pattern2, s.v) != 0)
            return logacterr( (fsfree(s), test_fclose(tf) ), TEST_FAILED, "Must be [%s] but not [%s]", pattern2, s.v);
    }

    fsfree(s);
    test_fclose(tf);
    return logret(TEST_PASSED, "done"); // TEST_FAILED
}

// -------------------------------------------------------------------

int
main(int argc, char *argv[])
{
    const char *logfilename = "log/fileutils.log";

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

