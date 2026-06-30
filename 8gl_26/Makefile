CC = gcc
CFLAGS = -I../utils -Wall -Wextra -std=gnu11 -g -O0 \
         -Wno-gnu-folding-constant -Wno-initializer-overrides \
         -fmacro-backtrace-limit=0 -fsanitize=address
LDFLAGS =
LDLIBS =

UTIL_PATH = ../utils

# ------------- Phony-алиасы для исполняемых файлов -------------
.PHONY: file_ops.p8.2 cp.p8.3 cat.8.1 files_checker mcp kaprekar triplets
.PHONY: all clean cleanlog cleanall no_targets__ list

file_ops.p8.2: out/file_ops.p8.2
cp.p8.3:       out/cp.p8.3
cat.8.1:       out/cat.8.1
files_checker: out/files_checker
mcp:           out/mcp
kaprekar:      out/kaprekar
triplets:      out/triplets

all: file_ops.p8.2 cp.p8.3 cat.8.1 files_checker mcp kaprekar triplets

# ------------- Исполняемые файлы (сборка в out/) -------------
# Цели, которым нужны ТОЛЬКО утилиты
out/file_ops.p8.2: src/file_ops.p8.2.c $(UTIL_PATH)/lib/cmutils.a
	$(CC) $(CFLAGS) $^ -o $@ $(LDLIBS)

out/cp.p8.3: src/cp.p8.3.c $(UTIL_PATH)/lib/cmutils.a
	$(CC) $(CFLAGS) $^ -o $@ $(LDLIBS)

out/cat.8.1: src/cat.8.1.c $(UTIL_PATH)/lib/cmutils.a
	$(CC) $(CFLAGS) $^ -o $@ $(LDLIBS)

out/kaprekar: src/kaprekar.c $(UTIL_PATH)/lib/cmutils.a
	$(CC) $(CFLAGS) $^ -o $@ $(LDLIBS)

out/triplets: src/triplets.c $(UTIL_PATH)/lib/cmutils.a
	$(CC) $(CFLAGS) $^ -o $@ $(LDLIBS)

# Цели, которым нужны утилиты + локальные объектные файлы
out/files_checker: src/files_checker.c lib/files.p8.5.o lib/proc_actions.o \
                   $(UTIL_PATH)/lib/cmutils.a
	$(CC) $(CFLAGS) $^ -o $@ $(LDLIBS)

out/mcp: src/mcp.c lib/files.p8.5.o $(UTIL_PATH)/lib/cmutils.a
	$(CC) $(CFLAGS) $^ -o $@ $(LDLIBS)

# ------------- Локальные объектные файлы (сразу в lib/) -------------
lib/files.p8.5.o: src/files.p8.5.c
	$(CC) $(CFLAGS) -c $< -o $@

lib/proc_actions.o: src/proc_actions.c
	$(CC) $(CFLAGS) -c $< -o $@

# ------------- Очистка -------------
clean:
	-rm -f src/*.c~ src/.*.un~
	-rm -rf out/*
	-rm -rf lib/*
	@find . -perm -u=x -type f -maxdepth 1 -exec rm {} \;

cleanlog:
	-rm -rf log/*

cleanall: clean cleanlog

no_targets__:
list:
	sh -c "$(MAKE) -p no_targets__ | awk -F':' '/^[a-zA-Z0-9][^\$$#\/\\t=]*:([^=]|$$)/ {split(\$$1,A,/ /);for(i in A)print A[i]}' | grep -v '__\$$' | sort"
