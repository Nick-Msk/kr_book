CC=gcc #clang
CFLAGS= -I../utils -Wall -Wextra -std=gnu11 -g -O0 -Wno-gnu-folding-constant -Wno-initializer-overrides
# -Wno-initializer-overrides -Wno-unused-function -Wno-format-zero-length
LDFLAGS=
LDLIBS=

all:

getint.5.1: src/getint.5.1.c lib/log.o lib/common.o lib/checker.o lib/array.o
	$(CC) $(CFLAGS)  $^ -o out/$@

getfloat.5.2: src/getfloat.5.2.c lib/log.o lib/common.o lib/checker.o lib/array.o
	$(CC) $(CFLAGS)  $^ -o out/$@

mystrcat.5.3: src/mystrcat.5.3.c lib/log.o lib/common.o lib/checker.o
	$(CC) $(CFLAGS)  $^ -o out/$@

mystrend.5.4: src/mystrend.5.4.c lib/log.o lib/common.o lib/checker.o
	$(CC) $(CFLAGS)  $^ -o out/$@

mystrings.5.5: src/mystrings.5.5.c lib/log.o lib/common.o lib/checker.o
	$(CC) $(CFLAGS)  $^ -o out/$@

days.5.8: src/days.5.8.c lib/log.o lib/common.o lib/checker.o
	$(CC) $(CFLAGS)  $^ -o out/$@

echo.p5.10: src/echo.p5.10.c
	$(CC) $(CFLAGS)  $^ -o out/$@

mygrep.p5.10: src/mygrep.p5.10.c lib/log.o lib/common.o lib/checker.o
	$(CC) $(CFLAGS)  $^ -o out/$@

tail.5.13: src/tail.5.13.c lib/log.o lib/common.o lib/checker.o lib/fileutils.o lib/fs.o lib/error.o
	$(CC) $(CFLAGS)  $^ -o out/$@

sortlines.p5.11: src/sortlines.p5.11.c lib/log.o lib/common.o lib/checker.o lib/fileutils.o lib/fs.o
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

lib/fileutils.o:
	cd $(UTIL_PATH) && $(MAKE) fileutils.o
	@cp $(UTIL_PATH)/lib/fileutils.o lib/

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

