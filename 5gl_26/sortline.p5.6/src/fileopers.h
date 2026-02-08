#ifndef _FILEOPERS_H
#define _FILEOPERS_H

/* ------------------------ Public API -------------------------- */

int             readlines(char *ptr[], int maxline);

int             writelines(const char *ptr[], int maxline);

int             freelines(const char *ptr[], int maxline);

#endif /* !_FILEOPERS_H */

