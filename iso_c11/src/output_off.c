#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#include <limits.h>

static const int        SAVER_MAX = 10;
static int              savers_fd[SAVER_MAX];
static int              saver_ptr = 0;

bool                    fu_disable(void);
bool                    fu_enable(void);

int                     main(void){

    printf("Slots: %d\n", getdtablesize());

    if (!fu_disable() )
        fprintf(stderr, "Unable to disable\n");

    int     a = 5, cnt = 0;
    fprintf(stdout, "out %d: a = %d\n", ++cnt, a);
    fprintf(stderr, "err: %d: a = %d\n", ++cnt, a);

    if (!fu_enable() )
        fprintf(stderr, "Unable to enable\n");

    a += 2;
    fprintf(stdout, "out: %d: a = %d\n", ++cnt, a);
    fprintf(stderr, "err: %d: a = %d\n", ++cnt, a);
    return 0;
}

static inline bool      fu_saver_put(int fd){
    if (saver_ptr < SAVER_MAX){
       savers_fd[saver_ptr++] = fd;
        return true;
    } else
        return false;
}
static inline bool      fu_saver_get(void){
    if (saver_ptr < SAVER_MAX)
       return savers_fd[saver_ptr++];
    else
        return INT_MAX;
}

bool                    fu_disable(void){

    int             dev_null_fd;
    fflush(0);

    if ( (dev_null_fd = open("/dev/null", O_WRONLY) ) < 0){
        perror("Unable to open dev/null");
        return false;
    }
    // semi-iterator
    int     *iterarr = (int []) {STDOUT_FILENO, STDERR_FILENO, INT_MAX};

    for (int fd = *iterarr; fd != INT_MAX; fd = *++iterarr) {
        int saver_fd;
        if (saver_ptr < SAVER_MAX){
            if ( (saver_fd = dup(fd) ) < 0 ){    //STDOUT_FILENO);
                perror("Unable to dup");    // "%d", fd);
                return false;
            }
            savers_fd[saver_ptr++] = saver_fd;
//        if (!fu_saver_put(saver_fd) )
  //          fprintf(stderr, "Out of savers...\n");
            if ( (dup2(dev_null_fd, fd) ) < 0){
                perror("Unable to dup2"); // "%d", fd);
                return false;
            }
        } else
            fprintf(stderr, "Out of savers...\n");
    }

    close(dev_null_fd);
    return true;
}

bool                    fu_enable(void){
    fflush(0);
    saver_ptr = 0;

    // semi-iterator, TODO: rework with struct { orig_fd = STDOUT_FILENO/STDOUT_FILENO, new_id
    int     *iterarr = (int []) {STDOUT_FILENO, STDERR_FILENO, INT_MAX};

    for (int fd = *iterarr; fd != INT_MAX; fd = *++iterarr) {
        int saver_fd = savers_fd[saver_ptr++]; //fu_saver_get();
        if ( (dup2(saver_fd, fd) ) < 0){  // not corrent but for now
            perror("Enable: Unable to dup2 out");   // "%d", fd);
            return false;
        }
        close(saver_fd);
    }
    return true;
}
