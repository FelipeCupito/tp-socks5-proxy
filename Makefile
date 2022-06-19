HEADERS=include
CLIENT=management_protocol
CLIENT_HEADERS=management_protocol/include
SRC=src
TEST=test/parser_tests


# Hay que hacer que compile con Clang tambien

HFILES=$(shell find $(HEADERS) -name '*.h' | sed 's/^.\///')
FILES=$(shell find $(SRC) -name '*.c' | sed 's/^.\///')
CLIENT_HFILES=management_protocol/include
CLIENT_FILES=$(wildcard management_protocol/*.c)
TESTFILES=$(shell find $(TEST) -name '*.c' | sed 's/^.\///')
# TESTFILES=$(wildcard tests/parser_tests/*.c)

CLIENT_OFILES=$(patsubst %.c,./%.o,$(CLIENT_FILES))
OFILES=$(patsubst %.c,./%.o,$(FILES))
CFLAGS = -g -Wall -Wextra -pedantic -pedantic-errors -O3 -std=c11 -D_POSIX_C_SOURCE=200112L  -Wno-unused-parameter -Wno-implicit-fallthrough $(MYCFLAGS)
LDFLAG = -lpthread
OTFILES=$(patsubst %.c,./%.o,$(TESTFILES))
# TESTFLAGS= -lcheck -lrt -lm -lsubunit -D_POSIX_C_SOURCE=200112L -std=c11 -pthread

%.o: %.c $(HFILES) $(CLIENT_HFILES)
	$(CC) -c -o $@ $< $(CFLAGS)

all:$(OFILES)
	$(CC) $(OFILES) $(CFLAGS) $(LDFLAG) -o server

client:$(CLIENT_OFILES)
	cd management_protocol; make all

test:$(OTFILES)
	cd test; make all

clean:
	cd test; make clean; cd ..; rm -rf $(OFILES) server


 #-Werror pasa los w a errores

.PHONY: all clean