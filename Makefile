HEADERS=include
SRC=src
TEST=test/parser_tests

# Hay que hacer que compile con Clang tambien
CC=gcc

HFILES=$(shell find $(HEADERS) -name '*.h' | sed 's/^.\///')
FILES=$(shell find $(SRC) -name '*.c' | sed 's/^.\///')
TESTFILES=$(shell find $(TEST) -name '*.c' | sed 's/^.\///')
# TESTFILES=$(wildcard tests/parser_tests/*.c)

OFILES=$(patsubst %.c,./%.o,$(FILES))
CFLAGS = -g -Wall -Wextra -pedantic -pedantic-errors -O3 -std=c11 -D_POSIX_C_SOURCE=200112L  -Wno-unused-parameter -Wno-implicit-fallthrough $(MYCFLAGS)
LDFLAG = -lpthread
OTFILES=$(patsubst %.c,./%.o,$(TESTFILES))
# TESTFLAGS= -lcheck -lrt -lm -lsubunit -D_POSIX_C_SOURCE=200112L -std=c11 -pthread

%.o: %.c $(HFILES)
	$(CC) -c -o $@ $< $(CFLAGS)

all:$(OFILES)
	$(CC) $(OFILES) $(CFLAGS) $(LDFLAG) -o server

test:$(OTFILES)
	cd test; make all

# clean:
	cd test; make clean	
	rm -rf $(OFILES)
    rm -rf $(OFILES) server

 #-Werror pasa los w a errores
.PHONY: all clean