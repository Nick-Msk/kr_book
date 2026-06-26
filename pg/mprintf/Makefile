PG_CONFIG = $(HOME)/mypg18.0/bin/pg_config

# Получаем нужные пути
INCLUDEDIR = $(shell $(PG_CONFIG) --includedir-server)
LIBDIR     = $(shell $(PG_CONFIG) --libdir)
SHAREDIR   = $(shell $(PG_CONFIG) --sharedir)/extension

# Цель по умолчанию — собрать .so
all: pg_printf.so

# Компиляция объектного файла
pg_printf.o: pg_printf.c
	gcc -I$(INCLUDEDIR) -fPIC -c $< -o $@

# Линковка разделяемой библиотеки
pg_printf.so: pg_printf.o
	gcc -shared -o $@ $^

# Установка (требует прав на запись в LIBDIR и SHAREDIR)
install: pg_printf.so
	cp pg_printf.so $(LIBDIR)/
	cp pg_printf.control $(SHAREDIR)/
	cp pg_printf--1.0.sql $(SHAREDIR)/

# Очистка
clean:
	rm -f pg_printf.o pg_printf.so

