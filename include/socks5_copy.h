#ifndef SOCKS5_COPY_H
#define SOCKS5_COPY_H

#include <errno.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdio.h>
#include <stdlib.h>     // calloc
#include <string.h>     // memcpy
#include <sys/socket.h> // socket
#include <sys/types.h>  // socket
#include <unistd.h>

#include "buffer.h"
#include "stm.h"
#include "selector.h"
#include "logger.h"
#include "socks5.h"
#include "metrics.h"
#include "pop3_sniffer.h"


void copy_init(const unsigned state, struct selector_key *key);
unsigned copy_read(struct selector_key *key);
unsigned copy_write(struct selector_key *key);

#endif
