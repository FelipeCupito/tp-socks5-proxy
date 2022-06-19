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


#define BUFFER_SIZE 4096 // TODO: sacar
#define ATTACHMENT(key) ((struct mng *)(key)->data)

/////////////////////////////////////////////////////////////////////////
// Estados posibles de cada estado de socks5
/////////////////////////////////////////////////////////////////////////
enum mng_state {
  HELLO_READ,

  HELLO_WRITE,

  REQUEST,

  REPLIES,

  DONE,

  ERROR,
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

  // buffer de escritura
  uint8_t raw_buff_a[BUFFER_SIZE], raw_buff_b[BUFFER_SIZE];
  buffer read_buffer, write_buffer;

  int toFree;
} mng;

#endif
