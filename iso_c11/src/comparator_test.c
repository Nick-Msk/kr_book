
// Comparators testing
// 1. test_str_equal_ne
// 2. test_str_ulike


#include <stdio.h>
#include <ctype.h>
#include <stdbool.h>
#include <log.h>

static bool		test_str_ulike(const char *source, int sz, const char *pattern);

static bool		test_str_equal_ne(const char *restrict source, int sz, const char *restrict pattern);

int
main(int argc, char *argv[]){
	LOG(const char *logfilename = "comparator.log");

	loginit(logfilename, false, 0, "Start");
	printf("%s:%d\n", *argv, argc);

	int		rs1 = 0, rs2 = 0;
	char	source1[] = "Abc\t\tde fgh\nj";
	char	source2[] = "QwertYuiopAsdfGhjKlZxcVbnm,./";
	char	source3[] = "abccccccccdefffffff";
	int 	sz1 = sizeof source1, sz2 = sizeof source2, sz3 = 14;	// 14 < sizeof source3

	if (!test_str_equal_ne(source1, sz1, "Abcdefghj"))
		logact(rs1 = 01, "Failed I test_str_equal_ne");

	if (test_str_equal_ne(source1, sz1, "Abcdefgh"))
		logact(rs1 = 01, "Failed II test_str_equal_ne");

	if (test_str_equal_ne(source1, sz1, "A     b   cde  fgh     "))
		logact(rs1 = 01, "Failed III test_str_equal_ne");

	printf("Tests test_str_equal_ne are %s\n", rs1 == 0 ? "passed" : "failed");

	if (!test_str_ulike(source2, sz2, "qwe"))
		logact(rs2 = 02, "Failed I test_str_ulike");

	if (!test_str_ulike(source2, sz2, "fghjkl"))
		logact(rs2 = 02, "Failed II test_str_ulike");

	if (test_str_ulike(source2, sz2, "fgf"))
		logact(rs2 = 02, "Failed III test_str_ulike");

	if (!test_str_ulike(source3, sz3, "cde"))
		logact(rs2 = 04, "Failed IV test_str_ulike");

	if (!test_str_ulike(source3, sz3, "cdef"))
        logact(rs2 = 04, "Failed V test_str_ulike");

	if (!test_str_ulike(source3, sz3, "ccc"))
        logact(rs2 = 04, "Failed VI test_str_ulike");

	if (test_str_ulike(source3, sz3, "defg"))
        logact(rs2 = 04, "Failed VII test_str_ulike");

	printf("Tests test_str_ulike are %s\n", rs2 == 0 ? "passed" : "failed");

	logclose("End with %d", rs1 | rs2);
	printf("Total Tests are %s\n", (rs1 | rs2) == 0 ? "passed" : "failed");

	return rs1 | rs2;
}


// ignore delimeters
// NOTE: will be checked separately
static bool
test_str_equal_ne(const char *restrict source, int sz, const char *restrict pattern)
{
	logenter("source [%s], pattern [%s], sz %d", source, pattern, sz);
    int 	i = 0, j = 0;
    while (i < sz && pattern[j])
    {
        char    s = source[i], p = pattern[j];
        if (isspace(s))
        {
            i++;
            continue;
        }
        if (isspace(p))
        {
            j++;
            continue;
        }
        if (s != p){     // find not equal symbols
			logmsg("NOT EQUAL: i = %d, j = %d, s = %c, p = %c", i, j, s, p);
            break;
		}
        i++, j++;
		logmsg("new i = %d (%c:%c), j = %d (%c:%c)", i, s, source[i], j, p, pattern[j]);
    }
	logret(false, "pattern[j=%d] = [%c], i = %d", j, pattern[j], i);
    // true if source and pattern are done
    return pattern[j] == '\0' && i == sz - 1;
}


static bool
test_str_ulike(const char *restrict source, int sz, const char *restrict pattern)
{
	logenter("source [%s], pattern [%s], sz %d", source, pattern, sz);
    int i = 0, j = 0, i_prev = -1;
    while (i < sz && pattern[j] != '\0')
    {
        char s = toupper(source[i]), p = toupper(pattern[j]);
        if (s != p)
        {
            j = 0;  //reset
			if (i_prev != -1)
            	i = i_prev;
            i_prev = -1;
        } else  // s == p
        {
            if (i_prev == -1)   // first time, save
                i_prev = i;
            j++;
        }
        i++;
    }

	logret(true, "pattern[j=%d] = [%c], i = %d", j, pattern[j], i);
    return pattern[j] == '\0';
}


