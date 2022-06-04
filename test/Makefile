CC=-D gcc

TESTFLAGS = -lcheck -lrt -lm -lsubunit -D_POSIX_C_SOURCE=200112L -std=c11 -pthread

SOURCES_PARSERS=$(wildcard parser_tests/*.c)

OBJECTS_PARSERS=$(wildcard SOURCES_PARSERS:.c=.o)

ALL_OBJECTS= $(OBJECTS_PARSERS)

%.o: %.c
	$(CC) $(TESTFLAGS) -I../include -c $< -o $@

all: parser_tests

parser_tests: hello_test
	$(CC) hello_test.c hello_test.o ../src/parsers/hello.c ../src/buffer.c -o hello_test.out $(TESTFLAGS)

clean: 
	rm -rf *.o *.out parser_tests/*.o parser_tests/*.out

.PHONY: all clean