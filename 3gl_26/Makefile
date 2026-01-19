CC=gcc #clang
CFLAGS= -I../utils -Wall -Wextra -std=gnu11 -g -O0 -Wno-gnu-folding-constant
# -Wno-initializer-overrides -Wno-unused-function -Wno-format-zero-length
LDFLAGS=
LDLIBS=

all:

binsearch.3.1: src/binsearch.3.1.c lib/log.o lib/metric.o lib/common.o lib/checker.o lib/array.o
	$(CC) $(CFLAGS)  $^ -o out/$@

escape.3.2: src/escape.3.2.c lib/log.o lib/common.o lib/checker.o
	$(CC) $(CFLAGS)  $^ -o out/$@

shellsort: src/shellsort.c lib/log.o lib/common.o lib/checker.o lib/array.o lib/metric.o
	$(CC) $(CFLAGS)  $^ -o out/$@

expand.3.3: src/expand.3.3.c lib/log.o lib/common.o lib/checker.o
	$(CC) $(CFLAGS)  $^ -o out/$@

UTIL_PATH = ../utils

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

# TODO: refactor to common:
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

