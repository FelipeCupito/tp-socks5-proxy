CFLAGS = -g -Wall -Wextra -pedantic -pedantic-errors -O3 -std=c11 -D_POSIX_C_SOURCE=200112L  -Wno-unused-parameter -Wno-implicit-fallthrough $(MYCFLAGS)
CFILES=$(wildcard *.c)
OFILES=$(CFILES:.c=.o)

all: $(OFILES)
	$(CC) $(CFILES) -o client $(CFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -I./include -c $< -o $@

clean:
	rm -rf $(OFILES)