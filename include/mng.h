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
#include "parsers/admin_connect.h"
#include "parsers/admin_get.h"
#include "parsers/admin_edit.h"
#include "parsers/admin_put.h"
#include "parsers/admin_delete.h"
#include "parsers/admin_configbuffsize.h"
#include "parsers/admin_configstatus.h"

enum request_actions{
  GET = 0x00,
  PUT = 0x01,
  EDIT = 0x02,
  CONFIGBUFFSIZE = 0x03,
  CONFIGSTATUS = 0x04,
  DELETE = 0x05
};

///////////////////////////////////////////
//CONNECT_READ
//////////////////////////////////////////
void mng_connect_read_init(const unsigned state, struct selector_key *key);
unsigned int mng_connect_read(struct selector_key *key);

///////////////////////////////////////////
//CONNECT_WRITE
//////////////////////////////////////////
unsigned int mng_connect_write(struct selector_key *key);

///////////////////////////////////////////
//REQUEST
//////////////////////////////////////////
unsigned int mng_request(struct selector_key *key);


////////////////////////////////////////////
//REQUEST_GET
////////////////////////////////////////////
void request_get_init(const unsigned state, struct selector_key *key);
unsigned int request_get_request(struct selector_key *key);

///////////////////////////////////////////
//REQUEST_PUT
//////////////////////////////////////////
void request_put_init(const unsigned state, struct selector_key *key);
unsigned int request_put_request(struct selector_key *key);

///////////////////////////////////////////
//REQUEST_EDIT
//////////////////////////////////////////
void request_edit_init(const unsigned state, struct selector_key *key);
unsigned int request_edit_request(struct selector_key *key);

///////////////////////////////////////////
//REQUEST_BUFFSIZE
//////////////////////////////////////////
void request_buffsize_init(const unsigned state, struct selector_key *key);
unsigned int request_buffsize_request(struct selector_key *key);

///////////////////////////////////////////
//REQUEST_CONFIGSTATUS
//////////////////////////////////////////
void request_configstatus_init(const unsigned state, struct selector_key *key);
unsigned int request_configstatus_request(struct selector_key *key);

///////////////////////////////////////////
//REQUEST_DELETE
//////////////////////////////////////////
void request_delete_init(const unsigned state, struct selector_key *key);
unsigned int request_delete_request(struct selector_key *key);


///////////////////////////////////////////
//REPLIES
//////////////////////////////////////////
unsigned int mng_replies(struct selector_key *key);


#endif
