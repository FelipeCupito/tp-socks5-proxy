CFLAGS = -g -Wall -Wextra -pedantic -pedantic-errors -O3 -std=c11 -D_POSIX_C_SOURCE=200112L  -Wno-unused-parameter -Wno-implicit-fallthrough $(MYCFLAGS)
CFILES=$(*.c)
HFILES=include
OFILES= args_handler.o management_protocol_handler.o management_protocol_client.o
%.o: %.c
	$(CC) $(CFLAGS) -I$(HFILES) -c $< -o $@

all: $(OFILES)
	$(CC) $(CFILES) -o client $(CFLAGS)