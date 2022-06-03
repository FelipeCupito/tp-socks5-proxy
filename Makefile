HEADERS=include
SRC=src
PARSERS=src/parsers

# Hay que hacer que compile con Clang tambien
CC=gcc

HFILES=$(shell find $(HEADERS) -name '*.h' | sed 's/^.\///')
FILES=$(shell find $(SRC) $(PARSERS) -name '*.c' | sed 's/^.\///')
# TESTFILES=$(wildcard tests/parser_tests/*.c)

OFILES=$(patsubst %.c,./%.o,$(FILES))
CFLAGS = -Wall -Wextra -pedantic -lpthread -pedantic-errors -O3 -g -std=c11 -D_POSIX_C_SOURCE=200112L $(MYCFLAGS)
# TESTFLAGS= -lcheck -lrt -lm -lsubunit -D_POSIX_C_SOURCE=200112L -std=c11 -pthread


%.o: %.c $(HFILES)
	$(CC) -c -o $@ $< $(CFLAGS)

all:$(OFILES)
	$(CC) $(OFILES) $(CFLAGS) -o server

test:
	cd test; make all

clean:
	cd test; make clean	
	rm -rf $(OFILES)

.PHONY: all clean


	
