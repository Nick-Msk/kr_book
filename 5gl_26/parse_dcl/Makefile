CC=gcc
CFLAGS= -I../../utils -Isrc/ -Wall -Wextra -std=gnu11 -g -O0 -Wno-gnu-folding-constant
# -Wno-initializer-overrides -Wno-unused-function -Wno-format-zero-length
LDFLAGS=
LDLIBS=

parse_dcl: src/parse_dcl.c lib/log.o lib/common.o lib/checker.o lib/dcl.o lib/token.o lib/fs.o lib/buffer.o lib/error.o 
	$(CC) $(CFLAGS)  $^ -o out/$@

parse_dcl2: src/parse_dcl.c lib/log.o lib/common.o lib/checker.o lib/dcl2.o lib/token.o lib/fs.o lib/buffer.o lib/error.o 
	$(CC) $(CFLAGS)  $^ -o out/$@

parse_dcl.o: src/parse_dcl.c
	$(CC) $(CFLAGS)  $^ -c

#fileopers: src/fileopers.c lib/log.o lib/test.o lib/error.o lib/checker.o lib/common.o
#	$(CC) $(CFLAGS) -DVARTESTING $^ -o test/$@_test

lib/dcl.o: src/dcl.c
	$(CC) $(CFLAGS)  $^ -c
	@mv dcl.o lib/

lib/dcl2.o: src/dcl2.c
	$(CC) $(CFLAGS)  $^ -c
	@mv dcl2.o lib/

lib/token.o: src/token.c
	$(CC) $(CFLAGS)  $^ -c
	@mv token.o lib/

UTIL_PATH = ../../utils

# libs from utils
lib/log.o:
	cd $(UTIL_PATH) && $(MAKE) log.o
	@cp $(UTIL_PATH)/lib/log.o lib/

lib/test.o:
	cd $(UTIL_PATH) && $(MAKE) test.o
	@cp $(UTIL_PATH)/lib/test.o lib/

lib/error.o:
	cd $(UTIL_PATH) && $(MAKE) error.o
	@cp $(UTIL_PATH)/lib/error.o lib/

lib/common.o:
	cd $(UTIL_PATH) && $(MAKE) common.o
	@cp $(UTIL_PATH)/lib/common.o lib/

lib/checker.o:
	cd $(UTIL_PATH) && $(MAKE) checker.o
	@cp $(UTIL_PATH)/lib/checker.o lib/

lib/metric.o:
	cd $(UTIL_PATH) && $(MAKE) metric.o
	@cp $(UTIL_PATH)/lib/metric.o lib/

lib/array.o:
	cd $(UTIL_PATH) && $(MAKE) array.o
	@cp $(UTIL_PATH)/lib/array.o lib/

lib/buffer.o:
	cd $(UTIL_PATH) && $(MAKE) buffer.o
	@cp $(UTIL_PATH)/lib/buffer.o lib/

lib/fs.o:
	cd $(UTIL_PATH) && $(MAKE) fs.o
	@cp $(UTIL_PATH)/lib/fs.o lib/

.PHONY: clean cleanlog cleanall no_targets__ list
clean:
	-rm src/*.c~
	-rm src/.*.un~
	-rm -rf out/*
	-rm lib/*
	@find . -perm -u=x -type f -ls -maxdepth 1 -exec rm {} \;

cleanlog:
	-rm log/*

cleanall: clean cleanlog

no_targets__:
list:
	sh -c "$(MAKE) -p no_targets__ | awk -F':' '/^[a-zA-Z0-9][^\$$#\/\\t=]*:([^=]|$$)/ {split(\$$1,A,/ /);for(i in A)print A[i]}' | grep -v '__\$$' | sort"

