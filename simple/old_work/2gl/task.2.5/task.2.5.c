#include <stdio.h>


int             any(const char *s, const char *pattern);

int             main(int argc, const char *argv[]){

    if (argc < 3){
        fprintf(stderr, "Usage: %s string pattern\n", *argv);
        return 1;
    }
    
    const char *s       = argv[1];
    const char *pattern = argv[2];
    
    int     pos = any(s, pattern);
    if (pos != -1)
        printf("Minimum position of [%s] in [%s] is %d\n",
            pattern, s, pos);
    else
        printf("Nothing of [%s] are not found in [%s]\n", pattern, s);

    return 0;
}


int             any(const char *s, const char *pattern){
     
    int     pos, found = 0;;
    for (pos = 0;  s[pos] != '\0'; pos++){
        found = 0;
        for (int j = 0; found == 0 && pattern[j] != '\0'; j++)
            if (s[pos] == pattern[j])
                found = 1;
        if (found)
            break;
    }
    if (!found)
        pos = -1;   // marker
    return pos;
}
