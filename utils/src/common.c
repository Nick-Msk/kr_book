#include "common.h"
#include "log.h"

// fill with 0.0 cnt elements 
void                cleaner_double(void *arr, int cnt)
{
	double *d = (double *) arr;
    for (int i = 0; i < cnt; i++)
        d[i] = 0.0;
}

// for now in common.c, then will be moved out
int			        get_line(char *line, int lim){
	int		c, i;
	for (i = 0; i < lim - 1 && (c = getchar()) != EOF && c != '\n'; i++)
		line[i] = c;
	if (c == '\n')
		line[i++] = c;
	line[i] = '\0';
	return i;
}

char                    *read_from_file(FILE *f, int *p_cnt){
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

char                      *uniq_str(char *s, int *p_len){
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

int                     fprint_bits(FILE *f, const char *str, unsigned val){
    char            buf[100];
    unsigned        pos = 0, bit;
    while (pos < sizeof(buf) - 1 && val > 0){ 
        bit = val & 0x1;
        buf[pos++] = bit + '0';
        val >>= 1;
    }   
    buf[pos] = '\0';
    reverse(buf, pos);
    return fprintf(f, "%s: %s\n", str, buf);
}

// reverse string
char*                   reverse(char *s, int len){
    logenter("[%s]", s);
    int i = 0, j = len - 1;
    while (i < j)
        char_exch(s + i++, s + j--);
    return logret(s, "Reversed: [%s]", s);
}

