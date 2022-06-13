TESTFLAGS = -lcheck -lrt -lm -lsubunit -D_POSIX_C_SOURCE=200112L -std=c11 -pthread

SOURCES_PARSERS=$(wildcard parser_tests/*.c)

OBJECTS_PARSERS=$(wildcard SOURCES_PARSERS:.c=.o)

ALL_OBJECTS= $(OBJECTS_PARSERS)

%.o: %.c
	$(CC) $(TESTFLAGS) -I../include -c $< -o $@

all: hello_test auth_test req_test admin_connect_test

hello_test: parser_tests/hello_test.o
	$(CC) parser_tests/hello_test.c ../src/parsers/hello.c ../src/buffer.c ../src/logger.c -o hello_test $(TESTFLAGS)

auth_test: parser_tests/auth_test.o
	$(CC) parser_tests/auth_test.c ../src/parsers/auth.c ../src/buffer.c ../src/logger.c -o auth_test $(TESTFLAGS)

req_test: parser_tests/request_test.o
	$(CC) parser_tests/request_test.c ../src/parsers/request.c ../src/buffer.c ../src/logger.c -o req_test $(TESTFLAGS)

admin_connect_test: parser_tests/admin_connect_test.o
	$(CC) parser_tests/admin_connect_test.c ../src/parsers/admin_connect.c ../src/buffer.c ../src/logger.c -o admin_connect_test $(TESTFLAGS)

clean: 
	rm -rf *.o *.out parser_tests/*.o parser_tests/*.out auth_test hello_test req_test admin_connect_test

.PHONY: all clean