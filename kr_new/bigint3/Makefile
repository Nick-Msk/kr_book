CC=gcc #clang
CFLAGS= -I../ -Wall -Wextra -std=gnu11 -g -O3
# -Wno-initializer-overrides -Wno-unused-function -Wno-format-zero-length
LDFLAGS=
LDLIBS=

all:

# main task - interactive test
interactive_run: interactive_run.c bigint.o log.o common.o error.o
	$(CC) $(CFLAGS) $^ -o $@_proj

bigint.o: bigint.c log.o common.o error.o
	$(CC) $(CFLAGS)  $^ -c	# __proj

# modular test for bigint
bigint:	bigint.c log.o common.o testing.o error.o
	$(CC) $(CFLAGS) -DBIGINTTESTING $^ -o $@_test

testing.o: ../testing.c ../testing.h
	$(CC) $(CFLAGS)  $^ -c

common.o: ../common.c ../common.h
	$(CC) $(CFLAGS)  $^ -c

log.o:	../log.c ../log.h
	$(CC) $(CFLAGS) $^ -c

error.o: ../error.c ../error.h
	$(CC) $(CFLAGS) $^ -c

# WO error fow now, for simplify
#error: error.o error.h log.c common.c	# can depends on log, not sure
#	$(CC) $(FLAGS) $^ -o $@_test

#error.o: error.c

.PHONY: clean cleanlog cleanall no_targets__ list
clean:
	-rm *.o
	-rm *_test
	-rm -r *.dSYM
	-rm *.gch
	find . -perm -u=x -type f -ls -maxdepth 1 -exec rm {} \;

cleanlog:
	-rm *.log

cleanall: clean cleanlog

no_targets__:
list:
	sh -c "$(MAKE) -p no_targets__ | awk -F':' '/^[a-zA-Z0-9][^\$$#\/\\t=]*:([^=]|$$)/ {split(\$$1,A,/ /);for(i in A)print A[i]}' | grep -v '__\$$' | sort"

