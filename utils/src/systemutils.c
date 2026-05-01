#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#include "systemutils.h"
#include "iterator.h"

static const int        SAVER_MAX = 10;
static int              g_savers_fd[SAVER_MAX];
static int              g_saver_ptr = 0;

bool                    su_stddisable(void){
    int             dev_null_fd;
    fflush(0);

    if ( (dev_null_fd = open("/dev/null", O_WRONLY) ) < 0){
        perror("Unable to open dev/null");
        return false;
    }
    foreachint (fd, STDOUT_FILENO, STDERR_FILENO) {
        printf("%d\n", fd);
    }
    // semi-iterator
    foreachint (fd, STDOUT_FILENO, STDERR_FILENO){
        int saver_fd;
        if (g_saver_ptr < SAVER_MAX){
            if ( (saver_fd = dup(fd) ) < 0 ){    //STDOUT_FILENO);
                perror("Unable to dup");    // "%d", fd);
                return false;
            }
            g_savers_fd[g_saver_ptr++] = saver_fd;
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

bool                    su_stdenable(void){
    fflush(0);
    g_saver_ptr = 0;

    foreachint (fd, STDOUT_FILENO, STDERR_FILENO){
        int saver_fd = g_savers_fd[g_saver_ptr++]; //fu_saver_get();
        if ( (dup2(saver_fd, fd) ) < 0){  // not corrent but for now
            perror("Enable: Unable to dup2 out");   // "%d", fd);
            return false;
        }
        close(saver_fd);
    }
    return true;
}

