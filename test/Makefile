TESTFLAGS = -lcheck -lrt -lm -lsubunit -D_POSIX_C_SOURCE=200112L -std=c11 -pthread

SOURCES_PARSERS=$(wildcard parser_tests/*.c)

OBJECTS_PARSERS=$(wildcard SOURCES_PARSERS:.c=.o)

ALL_OBJECTS= $(OBJECTS_PARSERS)

%.o: %.c
	$(CC) $(TESTFLAGS) -I../include -c $< -o $@

all: hello_test auth_test req_test admin_connect_test admin_action_test admin_get_test admin_put_test admin_edit_test admin_delete_test admin_configbuffsize_test admin_configstatus_test sniffer_test

hello_test: parser_tests/hello_test.o
	$(CC) parser_tests/hello_test.c ../src/parsers/hello.c ../src/buffer.c ../src/logger.c -o hello_test $(TESTFLAGS)

auth_test: parser_tests/auth_test.o
	$(CC) parser_tests/auth_test.c ../src/parsers/auth.c ../src/buffer.c ../src/logger.c -o auth_test $(TESTFLAGS)

req_test: parser_tests/request_test.o
	$(CC) parser_tests/request_test.c ../src/parsers/request.c ../src/buffer.c ../src/logger.c -o req_test $(TESTFLAGS)

admin_connect_test: parser_tests/admin_connect_test.o
	$(CC) parser_tests/admin_connect_test.c ../src/parsers/admin_connect.c ../src/buffer.c ../src/logger.c -o admin_connect_test $(TESTFLAGS)

admin_get_test: parser_tests/admin_get_test.o
	$(CC) parser_tests/admin_get_test.c ../src/parsers/admin_get.c ../src/buffer.c ../src/logger.c -o admin_get_test $(TESTFLAGS)

admin_put_test: parser_tests/admin_put_test.o
	$(CC) parser_tests/admin_put_test.c ../src/parsers/admin_put.c ../src/buffer.c ../src/logger.c -o admin_put_test $(TESTFLAGS)

admin_configbuffsize_test: parser_tests/admin_configbuffsize_test.o
	$(CC) parser_tests/admin_configbuffsize_test.c ../src/parsers/admin_configbuffsize.c ../src/buffer.c ../src/logger.c -o admin_configbuffsize_test $(TESTFLAGS)

admin_edit_test: parser_tests/admin_edit_test.o
	$(CC) parser_tests/admin_edit_test.c ../src/parsers/admin_edit.c ../src/buffer.c ../src/logger.c -o admin_edit_test $(TESTFLAGS)

admin_configstatus_test: parser_tests/admin_configstatus_test.o
	$(CC) parser_tests/admin_configstatus_test.c ../src/parsers/admin_configstatus.c ../src/buffer.c ../src/logger.c -o admin_configstatus_test $(TESTFLAGS)

admin_delete_test: parser_tests/admin_delete_test.o
	$(CC) parser_tests/admin_delete_test.c ../src/parsers/admin_delete.c ../src/buffer.c ../src/logger.c -o admin_delete_test $(TESTFLAGS)

sniffer_test: sniffer_test.o
	$(CC) sniffer_test.c ../src/pop3_sniffer.c ../src/buffer.c ../src/listgeneric.c ../src/logger.c -o sniffer_test $(TESTFLAGS)

clean: 
	rm -rf *.o *.out parser_tests/*.o parser_tests/*.out auth_test hello_test req_test admin_connect_test

.PHONY: all clean