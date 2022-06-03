CFLAGS = -lcheck -lrt -lm -lsubunit -D_POSIX_C_SOURCE=200112L -std=c11 -pthread

all: tests

tests: hello_test
# test_netutils
# buffer_test: buffer_test.o ../src/buffer.c
# 	$(CC) buffer_test.c -o buffer_test.out $(CFLAGS)

# auth_test: auth_test.o ../src/auth.c
# 	$(CC) auth_test.c ../src/auth.c ../src/buffer.c -o auth_test.out $(CFLAGS)

hello_test: hello_test.o
	$(CC) ./parser_tests/hello_test.c ../src/parsers/hello.c ../src/buffer.c -o hello_test.out $(CFLAGS)

# test_request: request_test.o 
# 	$(CC) request_test.c ../src/request.c ../src/buffer.c -o request_test.out $(CFLAGS)



clean: 
	rm -rf *.o *.out