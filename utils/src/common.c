#include "common.h"
#include "log.h"

/********************************************************************
                 COOMON MODULE IMPLEMENTATION
********************************************************************/

// --------------------------- API ---------------------------------

// fill with 0.0 cnt elements
void                            cleaner_double(void *arr, int cnt)
{
	double *d = (double *) arr;
    for (int i = 0; i < cnt; i++)
        d[i] = 0.0;
}

char                            *uniq_str(char *s, int *p_len){
    bool    hash[256] = {false};
    int     j = 0, i = 0;
    char    c;
    while ( (c = s[i++]) != '\0'){
        if (!hash[(int) c]){
            hash[(int) c] = true;
            s[j++] = c;
        }
    }
    s[j] = '\0';
    if (p_len)
        *p_len = j;
    return s; // logret(s, "new len = %d, new str[%s]", j, s);
}

// bits to string (STATIC for now) TODO: think about FastString usage here
extern const char               *bits_str(char *buf, int len, unsigned val){
    //logenter("val %u", val);
    int        pos = 0, bit;
    while (pos < len - 1 && val > 0){ 
        bit = val & 0x1;
        buf[pos++] = bit + '0';
        val >>= 1;
    }
    buf[pos] = '\0';
    reverse(buf, pos);
    return buf; //logret(buf, "[%s]", buf);
}

int                             fprint_bits(FILE *f, const char *str, unsigned val){
    char            buf[100];
    bits_str(buf, sizeof(buf), val);
    return fprintf(f, "%s: %s\n", str, buf);
}

// reverse string
char*                           reverse(char *s, int len){
    int i = 0, j = len - 1;
    while (i < j)
        char_exch(s + i++, s + j--);
    return s;
}

int                             fprintn(FILE *f, const char *str, int sz){
    char c;
    int i = 0;
    while (str && i < sz &&(c = str[i++]) != '\0')
        fputc(c, f);
    fputc('\n', f);
    return i;
}

// -------------------------------Testing --------------------------

#ifdef COMMONTESTING

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
        // TODO: 
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

    testenginestd(  // TODO:
        testnew(.f2 = tf1, .num = 1, .name = "SOMETHING simple test",                .desc = "", .mandatory=true)
    );

    logclose("end...");
    return 0;
}

#endif /* COMMONTESTING */

