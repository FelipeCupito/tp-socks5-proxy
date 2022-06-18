#ifndef SOCKS5_REQUEST_H
#define SOCKS5_REQUEST_H

#include <errno.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdio.h>
#include <stdlib.h>     // calloc
#include <string.h>     // memcpy
#include <sys/socket.h> // socket
#include <sys/types.h>  // socket
#include <unistd.h>
#include <pthread.h>

#include "buffer.h"
#include "stm.h"
#include "selector.h"
#include "logger.h"
#include "socks5.h"
#include "conn_logger.h"

///////////////////////////////////////////
//REQUEST_READ
//////////////////////////////////////////
void request_init(const unsigned state, struct selector_key *key);
unsigned int request_read(struct selector_key *key);

///////////////////////////////////////////
//REQUEST_RESOLV
//////////////////////////////////////////
unsigned int request_resolv_done(struct selector_key *key);


///////////////////////////////////////////
//REQUEST_CONNECTING
//////////////////////////////////////////

void request_connecting_init(const unsigned state, struct selector_key *key);
unsigned int request_connecting(struct selector_key *key);

///////////////////////////////////////////
//REQUEST_WRITE
//////////////////////////////////////////
unsigned int request_write(struct selector_key *key);

#endif