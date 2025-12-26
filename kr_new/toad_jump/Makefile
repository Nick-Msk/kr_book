CC=gcc #clang
CFLAGS= -I../ -Wall -Wextra -std=gnu11 -g -O3
# -Wno-initializer-overrides -Wno-unused-function -Wno-format-zero-length
LDFLAGS=
LDLIBS=

all:

toad_jump: toad_jump.c tmap.o log.o common.o iarray.o  # no testing module is required
	$(CC) $(CFLAGS) $^ -o $@_proj

tmap: tmap.c log.o common.o testing.o
	$(CC) $(CFLAGS) -DTMAPTESTING $^ -o $@_test

tmap.o:	tmap.c log.o common.o
	$(CC) $(CFLAGS)  $^ -c

iarray.o: ../iarray.c ../iarray.h
	$(CC) $(CFLAGS)  $^ -c

testing.o: ../testing.c
	$(CC) $(CFLAGS)  $^ -c

common.o: ../common.c ../common.h
	$(CC) $(CFLAGS)  $^ -c

log: log.o ../testing.c ../common.o			# not sure
	$(CC) $(CFLAGS) -DLOGTESTING $^ -o $@_test

log.o:	../log.c ../log.h
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

