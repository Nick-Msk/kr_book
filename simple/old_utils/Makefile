CC=gcc #clang
CFLAGS= -I./ -Wall -Wextra -std=gnu11 -g -O0 -Wno-initializer-overrides
# -Wno-initializer-overrides -Wno-unused-function -Wno-format-zero-length
LDFLAGS=
LDLIBS=

all:

checker: checker.c log.o testing.o error.o
	$(CC) $(CFLAGS) -DCHECKERTESTING $^ -o $@_test

checker.o: checker.c log.c
	$(CC) $(CFLAGS) $^ -c

iarray: iarray.c log.c testing.o
	$(CC) $(CFLAGS) -DIARRAYTESTING $^ -o $@_test

iarray.o: iarray.c log.c
	$(CC) $(CFLAGS) $^ -c

error: error.c log.o common.o testing.o
	$(CC) $(CFLAGS) -DERRORTESTING $^ -o $@_test

error.o: error.c log.o common.o
	$(CC) $(CFLAGS)  $^ -c

log: log.c testing.o error.o
	$(CC) $(CFLAGS) -DLOGTESTING $^ -o $@_test

log.o:	log.c
	$(CC) $(CFLAGS) $^ -c

testing: testing.c error.o log.o
	$(CC) $(CFLAGS) -DTESTTESTING $^ -o $@_test

iter: iter.c log.o common.o error.o testing.o
	$(CC) $(CFLAGS) -DITERTESTING $^ -o $@_test

iter.o: iter.c log.o common.o error.o
	$(CC) $(CFLAGS)  $^ -c


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

