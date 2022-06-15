#ifndef SOCKS5_AUTH_H
#define SOCKS5_AUTH_H

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





///////////////////////////////////////////
//AUTH_READ
//////////////////////////////////////////
void auth_read_init(const unsigned state, struct selector_key *key);
unsigned int auth_read(struct selector_key *key);


///////////////////////////////////////////
//AUTH_WRITE
//////////////////////////////////////////

unsigned int auth_write(struct selector_key *key);



#endif