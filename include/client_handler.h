#ifndef CLIENT_HANDLER_H
#define CLIENT_HANDLER_H

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
#include "stm.h"
#include  "socks5.h"

void client_passive_accept(struct selector_key *key);

void socks5_read(struct selector_key *key);
void socks5_write(struct selector_key *key);
void socks5_block(struct selector_key *key);
void socks5_close(struct selector_key *key);

static const struct fd_handler socks5_handler = {
    .handle_read = socks5_read,
    .handle_write = socks5_write,
    .handle_close = socks5_close,
    .handle_block = socks5_block,
};

#endif
