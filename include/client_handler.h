
#include <stdio.h>
#include <stdlib.h> // calloc
#include <string.h> // memcpy
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>   // socket
#include <sys/socket.h>  // socket
#include <netinet/in.h>
#include <netinet/tcp.h>

#include "buffer.h"
#include "selector.h"
#include "logger.h"


void client_passive_accept(struct selector_key *key);
void client_read(struct selector_key *key);
void client_write(struct selector_key *key);
void client_block(struct selector_key *key);
void client_close(struct selector_key *key);


static const struct fd_handler client_handler = {
    .handle_read = client_read,
    .handle_write = client_write,
    .handle_close = client_close,
    .handle_block = client_block,
};