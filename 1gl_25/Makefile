CC=gcc #clang
CFLAGS= -I../ -Wall -Wextra -std=gnu11 -g -O0
# -Wno-initializer-overrides -Wno-unused-function -Wno-format-zero-length
LDFLAGS=
LDLIBS=

all:

hello: src/hello.c
	$(CC) $(CFLAGS) $^ -o $@

test_pi: src/test_pi.c
	$(CC) $(CFLAGS) $^ -o out/$@

test1: src/test1.c
	$(CC) $(CFLAGS) $^ -o out/$@

#common.o: ../common.c ../common.h
#	$(CC) $(CFLAGS)  $^ -c

#log: log.o ../testing.c ../common.o			# not sure
#	$(CC) $(CFLAGS) $^ -o $@_test

#log.o:	../log.c ../log.h
#	$(CC) $(CFLAGS) -DLOGTESTING $^ -c

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
	-rm out/*
	find . -perm -u=x -type f -ls -maxdepth 1 -exec rm {} \;

cleanlog:
	-rm log/*

cleanall: clean cleanlog

no_targets__:
list:
	sh -c "$(MAKE) -p no_targets__ | awk -F':' '/^[a-zA-Z0-9][^\$$#\/\\t=]*:([^=]|$$)/ {split(\$$1,A,/ /);for(i in A)print A[i]}' | grep -v '__\$$' | sort"

