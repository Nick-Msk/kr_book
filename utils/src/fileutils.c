
#include "log.h"
#include "common.h"
#include "fs.h"
#include "fileutils.h"

/********************************************************************
                 FILEUTILS MODULE IMPLEMENTATION
********************************************************************/

static const int                FU_LINE_CNT = 100;

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
int                             fgetline_fs(FILE *restrict in, fs *restrict s){
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

// return fs for WHOLE file!
fs                              readfs_file(FILE *f){
    logenter("%p", f);
    int     len, cnt = 1024;   // just piece of data
    long    pos = 0L;
    fs      str = fsinit(cnt);
    while ( (len = fread(fsstr(str) + pos, 1, cnt - 1, f) ) > 0){
        pos += len;
        logsimple("pos %ld, len %d sz %d", pos, fslen(str), fssz(str) );
        fsincrease(str, cnt);
    }
    fsetlen(str, pos);
    return logret(str, "%ld bytes were read", pos);
}

// read all (if maxline == -1 or maxline) lines separately into fs[]
int                             freadlines(FILE *restrict f, fs **restrict ptr){
    logenter("file: %p, fs **: %p", ptr, ptr);
    
    if (!ptr)
        return logerr(-1, "Zero ptr");
    int     nlines = 0, len, maxlines = FU_LINE_CNT;
    fs      line = fsempty();
    fs     *lines = malloc(maxlines * sizeof(fs));  // 
    if (!lines){
        perror("Unable to allocated");      // sysraisesig? TODO:
        return logerr(-1, "Unable to allocated %lu", maxlines * sizeof(fs) );
    }

    while ( (len = fgetline_fs(f, &line) ) > 0){
        if (nlines >= maxlines){
            maxlines += FU_LINE_CNT;
            fs *tmp = lines;
            if ( (tmp = realloc(tmp, maxlines * sizeof(fs) ) ) == 0){
                perror("Unable to allocated");      // sysraisesig? TODO:
                *ptr = lines;
                return logret(nlines, "Return Only %d lines, because unable to allocate more", nlines);
            } else
                lines = tmp;
        }
        lines[nlines++] = line;   // move to array
        line = fsempty();
    }
    // no need to free fs line, it's .v=0 after last cycle (or if no one cycle)
    *ptr = lines;
    return logret(nlines, "Total %d", nlines);
}

// write cnt fs into stream
int                             fwritelines(FILE *restrict f, const fs *restrict lines, int max){
    int     cnt = 0;
    while (max-- > 0){
        const fs *pt = lines++;
        cnt += fprintf(f, "%s", pt->v);
    }
    return cnt;
}

void                            freelines(fs *lineptr, int nlines){
    while (nlines-- > 0)
        fs_free(lineptr++);
}

/*int             readlines(char *ptr[], int maxline){
    int     nlines = 0, len;
    char   *p, line[MAXLEN];

    while ( (len = get_line(line, maxline) ) > 0)
        if (nlines >= maxline){
            fprintf(stderr, "maxline (%d) were execced\n", maxline);
            break;
        } else if ( (p = malloc(len + 1)) == 0){ 
            fprintf(stderr, "Unable to allocated %d\n", len + 1); 
            break;
        } else {
            strcpy(p, line);
            ptr[nlines++] = p;
        }   
    return nlines;
}*/


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

// ------------------------- TEST 2 ---------------------------------

static TestStatus
tf2(const char *name)
{
    logenter("%s", name);
    TFILE   tf;
    fs      s = fsempty();
    const char *fname = "test/util.out";

    int         subnum = 0;
    {
        test_sub("subtest %d", ++subnum);

        char    pattern1[] = "Test pattern ^&*()!@";
        char    pattern2[] = "second";

        tf = test_fopen("/tmp/");

        for (int i = 0; i < 50; i++){
            fprintf(tfile(tf), "%d\t: %s\n", i, pattern1);
            fprintf(tfile(tf), "%d\t: %s\n", i, pattern2);
        }
        test_freset(tf);
        s = readfs_file(tfile(tf));
        if (!fs_validate(stdout, &s) ){
            return logacterr( (fsfree(s), test_fclose(tf) ), TEST_FAILED, "Validation failed");
        }
        {
            FILE *f = fopen(fname, "w+");
            if (!f){
                return logacterr( (fsfree(s), test_fclose(tf) ), TEST_FAILED,  "Unable to open %s, please check perms", fname);
            }
            FS_TECH_PRINT_COUNT = 200;  // for fs_techfprint
            fstechfprint(f, s);
            fwrite(fsstr(s), 1, s.len, f);
            fclose(f);
        }
    }

    fsfree(s);
    test_fclose(tf);
    printf("Please check output file %s\n", fname);
    return logret(TEST_MANUAL, "Please check output file %s", fname); // TEST_FAILED
}

// ------------------------- TEST 3 ---------------------------------

static TestStatus
tf3(const char *name)
{
    logenter("%s", name);
    TFILE   tf;

    int         subnum = 0;
    {
        test_sub("subtest %d", ++subnum);

        fs      pt1 = fsliteral("Test pattern ^&*()!@\n");
        fs      pt2 = fsliteral("second\n");
        fs      pt3 = fsliteral("thrid...\n");
        fs      pt4 = fsliteral("4........\n");
        fs      pts[] = {pt1, pt2, pt3, pt4};

        tf = test_fopen("/tmp/");
        // write
        fwritelines(tfile(tf), pts, COUNT(pts) );
        test_freset(tf);
        freelines(pts, COUNT(pts) );    // that is useless (because of literals) but it must work!

        // read
        fs *pts2;
        int cnt = freadlines(tfile(tf), &pts2);
        if (cnt != COUNT(pts))
            return logacterr( (freelines(pts2, cnt), test_fclose(tf) ), TEST_FAILED,  "Count = %d but it should be %d", cnt, COUNT(pts));

        test_fclose(tf);
        test_sub("subtest %d", ++subnum);
        for (int i = 0; i < COUNT(pts); i++){
            logauto(i);
            fstechfprint(logfile, pts[i]);
            fstechfprint(logfile, pts2[i]);
            if (fscmp(pts[i], pts2[i]) != 0)
                return logacterr( (freelines(pts2, cnt), test_fclose(tf) ), TEST_FAILED,  "Line %d = [%s] but it should be [%s]", i, pts2[i].v, pts[i].v);
        }
        freelines(pts2, cnt);
    }

    return logret(TEST_PASSED, "done"); // TEST_FAILED
}

// -------------------------------------------------------------------

int
main( /*int argc, const char *argv[] */ )
{
    logsimpleinit("Start");

    /* const char *logfilename = "log/fileutils.log";
    if (argc > 1)
        logfilename = argv[1];
    loginit(logfilename, false, 0, "Starting"); */

    testenginestd(
        testnew(.f2 = tf1, .num = 1, .name = "Getline_fs() simple test",                .desc = "", .mandatory=true)
      , testnew(.f2 = tf2, .num = 2, .name = "Readfs_file() simple test",               .desc = "", .mandatory=true)
      , testnew(.f2 = tf3, .num = 3, .name = "Realline/writeline simple test",          .desc = "", .mandatory=true)
    );

    logclose("end...");
    return 0;
}

#endif /* FILEUTILSTESTING */
