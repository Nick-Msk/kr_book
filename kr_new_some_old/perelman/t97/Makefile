CC=gcc #clang
CFLAGS= -I../.. -Wall -Wextra -std=gnu11 -g -O0
# -Wno-initializer-overrides -Wno-unused-function -Wno-format-zero-length
LDFLAGS=
LDLIBS=

simple: simple.c  log.o error.o
	$(CC) $(CFLAGS)  $^ -o $@_proj

iter_api: iter_api.c log.o error.o iter.o
	$(CC) $(CFLAGS)  $^ -o $@_proj

common.o: ../../common.c ../../common.h
	$(CC) $(CFLAGS)  $^ -c

log.o:	../../log.c ../../log.h
	$(CC) $(CFLAGS) $^ -c

error.o: ../../error.c ../../error.h
	$(CC) $(CFLAGS) $^ -c

iter.o: ../../iter.c ../../iter.h log.o
	$(CC) $(CFLAGS) $^ -c

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


