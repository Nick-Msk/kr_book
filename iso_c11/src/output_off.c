#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>

static int              saver_fd;
bool                    fu_disable(FILE *);
bool                    fu_enable(FILE *);

int             main(void){
    printf("Slots: %d\n", getdtablesize());
    if (!fu_disable(stdout) )
        fprintf(stderr, "Unable to disable\n");
    int     a = 5, cnt = 0;
    printf("%d: a = %d\n", ++cnt, a);

    if (!fu_enable(stdout) )
        fprintf(stderr, "Unable to enable\n");
    a += 2;
    printf("%d: a = %d\n", ++cnt, a);
    return 0;
}

bool            fu_disable(FILE *f){
    fflush(f);
    int             fd = fileno(f);
    int             dev_null_fd;

    if ( (saver_fd = dup(fileno(f) ) ) < 0 ){    //STDOUT_FILENO);
        perror("Unable to dup");
        return false;
    }
    if ( (dev_null_fd = open("/dev/null", O_WRONLY) ) < 0){
        perror("Unable to open dev/null");
        return false;
    }
    if ( (dup2(dev_null_fd, fd) ) < 0){
        perror("Unable to dup2");
        return false;
    }
    close(dev_null_fd);
    return true;
}

bool            fu_enable(FILE *f){
    if ( (dup2(saver_fd, STDOUT_FILENO) ) < 0){  // not corrent but for now
        perror("En: Unable to dup2");
        return false;
    }
    close(saver_fd);
    return true;
}
