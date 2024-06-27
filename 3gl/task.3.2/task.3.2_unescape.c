#include <stdio.h>

#define             MAXLEN      8192

int                 unescape(char *s, int sz, const char *t);
int                 get_line(char *line, int lim);

int                 main(void){

    char        buf[MAXLEN], target[MAXLEN];
    int         len;

    while ((len = get_line(buf, MAXLEN)) > 0){
        unescape(target, sizeof(target), buf);
        printf("%s", target);
    }
    return 0;
}

int                 unescape(char *s, int sz, const char *t){
    int     i = 0, j = 0;
    char    c;
    while (i < sz - 1 && t[j] != '\0')
        switch (t[j]){
            case '\\':
                switch (c = t[++j]){
                    case 'n':
                        s[i++] = '\n'; j++;
                    break;
                    case 't':
                        s[i++] = '\t'; j++;
                    break;
                    case '\\':
                        s[i++] = '\\'; j++;
                    break;
                    default:
                        s[i++] = '\\';
                    break;
                }
            break;
            default:
                s[i++] = t[j++];
            break;
        }
    s[i] = '\0';
    return i;
}

int			get_line(char *line, int lim){
	int		c, i;
	for (i = 0; i < lim - 1 && (c = getchar()) != EOF && c != '\n'; i++)
		line[i] = c;
	if (c == '\n')
		line[i++] = c;	
	line[i] = '\0';

	return i;
}


