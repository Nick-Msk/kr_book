#ifndef _BUFFER_H_
#define _BUFFER_H_

/****************************************** LOW LEVEL ***************************************************/
int                 getch(void);
int                 ungetch(int);
int                 ungets(const char *s);

#endif /* _BUFFER_H_ */

