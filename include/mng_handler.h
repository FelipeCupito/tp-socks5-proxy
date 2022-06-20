#ifndef MNG_HANDLER_H
#define MNG_HANDLER_H

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
#include "logger.h"
#include "selector.h"
#include "stm.h"
#include "mng.h"
#include "parsers/admin_connect.h"
#include "parsers/admin_get.h"
#include "parsers/admin_edit.h"
#include "parsers/admin_put.h"
#include "parsers/admin_delete.h"
#include "parsers/admin_configbuffsize.h"
#include "parsers/admin_configstatus.h"


#define BUFFER_SIZE 4096 // TODO: sacar
#define ATTACH(key) ((struct mng *)(key)->data)

/////////////////////////////////////////////////////////////////////////
// FD HANDLER
/////////////////////////////////////////////////////////////////////////
const struct fd_handler mng_passive_handler;
const struct fd_handler mng_handler;

/////////////////////////////////////////////////////////////////////////
// Estados posibles de cada estado de socks5
/////////////////////////////////////////////////////////////////////////
enum mng_state {
  CONNECT_READ,
  CONNECT_WRITE,
  REQUEST,
  REQUEST_GET,
  REQUEST_PUT,
  REQUEST_EDIT,
  REQUEST_BUFFSIZE,
  REQUEST_CONFIGSTATUS,
  REQUEST_DELETE,
  REPLIES,
  MNG_DONE,
  MNG_ERROR,
};
/////////////////////////////////////////////////////////////////////////
// Store de cada estado
/////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////
// struc de cada socks5
////////////////////////////////////////////////////////////////////////
typedef struct mng {
  struct sockaddr_storage client_addr;
  socklen_t client_addr_len;
  int client_fd; // fd del socket del cliente

  // Estado del Socket:
  struct state_machine stm;

  // estado del socket cliente
  union parsers {
    struct admin_connect_parser connect;
    struct admin_get_parser get;
  } parsers;

  // buffer de escritura
  uint8_t raw_buff_a[BUFFER_SIZE], raw_buff_b[BUFFER_SIZE];
  buffer read_buffer, write_buffer;

  int close;
} mng;

#endif
