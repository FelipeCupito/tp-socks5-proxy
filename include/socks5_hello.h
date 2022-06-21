#ifndef SOCKS5_HELLO_H
#define SOCKS5_HELLO_H

#include <errno.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdio.h>
#include <stdlib.h>     // calloc
#include <string.h>     // memcpy
#include <sys/socket.h> // socket
#include <sys/types.h>  // socket
#include <unistd.h>

//#include "buffer.h"
//#include "stm.h"
//#include "selector.h"
//#include "logger.h"
#include "socks5.h"
//#include "parsers/hello.h"
#include "server_config.h"


///////////////////////////////////////////
//HELLO_READ
//////////////////////////////////////////
void hello_read_init(const unsigned state, struct selector_key *key);
unsigned int hello_read(struct selector_key *key);


///////////////////////////////////////////
//HELLO_WRITE
//////////////////////////////////////////
unsigned int hello_write(struct selector_key *key);

#endif
