HEADERS=include
SRC=src

HFILES=$(shell find $(HEADERS) -name '*.h' | sed 's/^.\///')
FILES=$(shell find $(SRC) -name '*.c' | sed 's/^.\///')

OFILES=$(patsubst %.c,./%.o,$(FILES))
CFLAGS = -g -Wall -Wextra -pedantic -pedantic-errors -O3 -std=c11 -D_POSIX_C_SOURCE=200112L  -Wno-unused-parameter -Wno-implicit-fallthrough $(MYCFLAGS)
LDFLAG = -lpthread

%.o: %.c $(HFILES)
	$(CC) -c -o $@ $< $(CFLAGS)

all:$(OFILES)
	$(CC) $(OFILES) $(CFLAGS) $(LDFLAG) -o server


clean:	
	rm -rf $(OFILES) server

 #-Werror pasa los w a errores