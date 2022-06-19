#ifndef MNG_H
#define MNG_H

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
#include "mng_handler.h"
#include "server_config.h"
#include "/parsers/admin_connect.h"


///////////////////////////////////////////
//HELLO_READ
//////////////////////////////////////////
void mng_hello_read_init(const unsigned state, struct selector_key *key);
unsigned int mng_hello_read(struct selector_key *key);

///////////////////////////////////////////
//HELLO_WRITE
//////////////////////////////////////////
unsigned int mng_hello_write(struct selector_key *key);

///////////////////////////////////////////
//REQUEST
//////////////////////////////////////////
void mng_request_init(const unsigned state, struct selector_key *key);
unsigned int mng_request(struct selector_key *key);

///////////////////////////////////////////
//REPLIES
//////////////////////////////////////////
unsigned int mng_replies(struct selector_key *key);


#endif