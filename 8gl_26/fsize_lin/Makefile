CC=gcc #clang
CFLAGS= -Isrc/ -Wall -Wextra -std=gnu11 -g -O0 -Wno-gnu-folding-constant -Wno-initializer-overrides 
# -fmacro-backtrace-limit=0
# -Wno-initializer-overrides -Wno-unused-function -Wno-format-zero-length
LDFLAGS=
LDLIBS=

all:

fsize.p8.6: src/fsize.p8.6.c lib/fsize.o lib/filelib.o
	$(CC) $(CFLAGS)  $^ -o out/$@

lib/fsize.o: src/fsize.c
	$(CC) $(CFLAGS) -I./src  $^ -c
	@mv fsize.o lib/

lib/filelib.o: src/filelib.c
	$(CC) $(CFLAGS) -I./src  $^ -c
	@mv filelib.o lib/

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

