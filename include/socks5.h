#ifndef SOCKS_5_H
#define SOCKS_5_H

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
#include "parsers/auth.h"
#include "parsers/hello.h"
#include "parsers/request.h"
#include "socks5_auth.h"
#include "socks5_copy.h"
#include "socks5_hello.h"
#include "socks5_request.h"


#define MAX_IPS 10
#define IP_V4_ADDR_SIZE 4
#define IP_V6_ADDR_SIZE 16
#define PORT_SIZE 2
#define BUFFER_SIZE 4096 // TODO: sacar

#define ATTACHMENT(key) ((struct socks5 *)(key)->data)

struct socks5 *socks5_new(const int client, struct sockaddr_storage* clntAddr, socklen_t clntAddrLen);

/////////////////////////////////////////////////////////////////////////
// FD HANDLER
/////////////////////////////////////////////////////////////////////////
const struct fd_handler socks5_passive_handler;
const struct fd_handler socks5_handler;

/////////////////////////////////////////////////////////////////////////
// Estados posibles de cada estado de socks5
/////////////////////////////////////////////////////////////////////////
enum socks_state {

  /*
   * recibe el mensaje "hello" del cliente y lo procesa
   *
   * Intereses:
   *    - OP_READ sobre client_fd
   *
   * Transiciones:
   *    - HELLO_READ mietras el mesaje no este completo
   *    - HELLO_WRITE cuando esta completo
   *    - ERROR ante cualquier error
   */
  HELLO_READ,

  /*
   * envia la respuesta del "hello" al cliente
   *
   * Intereses:
   *    - OP_WRITE sobre client_fd
   *
   * Transiciones:
   *    - HELLO_WRITE mietras queden bytes por enviar
   *    - REQUEST_READ cuando se enviaron todos los bytes
   *    - ERROR ante cualquier error
   */
  HELLO_WRITE,

  /*
   * state when receiving the user and password
   *
   * Interests:
   *  -OP_READ -> Read the info sent by the user
   *
   * Transitions:
   *  - USSERPASS_READ -> While there are bytes being read
   *  - USSERPASS_WRITE -> When all the bytes have been read and processed
   *  - ERROR -> In case of an error
   */
  AUTH_READ,

  /*
   * State when receiving the user and password
   *
   * Interests:
   *  -OP_WRITE -> Write if u+p is valid or not
   *
   * Transitions:
   *  - USSERPASS_READ -> While there are bytes being sent
   *  - REQUEST_READ -> If u+p is valid
   *  - ERROR -> In case of u+p invalid or other error
   */
  AUTH_WRITE,

  /*
   * recibe el mesaje "request" del cliente y inicia su procesamiento
   *
   * Intereses:
   *    - OP_READ sobre client_fd
   *
   * Transiciones:
   *    - REQUEST_READ mietras el mesaje no este completo
   *    - REQUEST_RESOLV si me pasan un dominio
   *    - REQUEST_CONNECTING iniciar coneccion
   *    - ERROR ante cualquier error
   */
  REQUEST_READ,

  /*
   * espera la resolucion del DNS
   *
   * Intereses:
   *    - OP_NOOP sobre client_fd
   *
   * Transiciones:
   *    - REQUEST_CONNECTING si re resuelve el nombre y se puede iniciar la
   * conexion
   *    - REQUEST_WRITE en otro caso
   */
  REQUEST_RESOLV,

  /*
   * Espera que se establezca la conecxion al server final
   *
   * Intereses:
   *    - OP_WRITE sobre client_fd
   *
   * Transiciones:
   *    - (algo) si hay mas ip paraconectar
   *    - REQUEST_WRITE si no hay mas ip para probar
   */
  REQUEST_CONNECTING,

  /*
   * Envia la repuesta del "request" al cliente
   *
   * Intereses:
   *    - OP_WRITE sobre client_fd
   *    - OP_NOOP sobre origin_fd
   *
   * Transiciones:
   *    - REQUEST_WRITE mitras queben bytes por enviar
   *    - COPY si el reques se envio y tenemos que copiar en otro caso
   *    - ERROR
   */
  REQUEST_WRITE,

  /*
   * Copia bytes entre client_fd y origin_fd
   *
   * Interests:
   *  - OP_READ si hay espacio para escribir en el buffer de lectura
   *   - OP_WRITE si hay bytes para leer en el buffer de ecritura
   *
   * Transitions:
   *  - DONE
   **/
  COPY,

  DONE,

  ERROR,
};
/////////////////////////////////////////////////////////////////////////
// Estados posibles de cada estado de socks5
/////////////////////////////////////////////////////////////////////////

typedef enum addr_type {
  IPv4 = 0x01,
  DOMAINNAME = 0x03,
  IPv6 = 0x04,
} addr_type;

/////////////////////////////////////////////////////////////////////////
// Store de cada estado
/////////////////////////////////////////////////////////////////////////

typedef struct hello_data {

  buffer *rb, *wb;
  struct hello_parser parser;
  uint8_t method;

} hello_data;

typedef struct auth_data {
  int i;
  // TODO;
} auth_data;

typedef struct request_data {

  buffer *rb, *wb;

  // parser
  struct request request;
  struct request_parser parser;
  // resumen de la respuesta a enviar
  enum socks_response_status status;

  // a donde nos tenemos que conectar
  struct sockaddr_storage *final_server_addr;
  socklen_t *final_server_len;
  int server_domain;

  // fd
  const int *client_fd;
  int *final_server_fd;

} request_data;

typedef struct resolv_data {
  addr_type addrType; // guardo el tipo de conexion

  char ipv4_addrs[MAX_IPS][IP_V4_ADDR_SIZE];
  uint8_t ipv4_size; // cantidad de ipv4, -1 si no hay

  char ipv6_addrs[MAX_IPS][IP_V6_ADDR_SIZE];
  uint8_t ipv6_size; // cantidad de ipv6,

  // server port
  uint8_t port[PORT_SIZE];

  // en caso que tengamos que resolver
  uint8_t *resolve_addr;
  uint8_t resolve_addr_len;

  // Buffers de escritura y lectura
} resolv_data;

typedef struct connecting_data {
  // write buffer
  buffer *wb;
  int *final_server_fd;
  int *client_fd;
  // enum connection_state status;
} connecting_data;

typedef struct copy_data {
  /** File descriptor */
  int *fd;

  /** buffers para hacer la copia **/
  buffer *rb, *wb;

  // escritura o lect
  fd_interest interest;

  // apunta al otro copy_data
  struct copy_data *other_copy;

} copy_data;

////////////////////////////////////////////////////////////////////////
// struc de cada socks5
////////////////////////////////////////////////////////////////////////
typedef struct socks5 {

  // cliente:
  struct sockaddr_storage client_addr;
  socklen_t client_addr_len;
  int client_fd; // fd del socket del cliente

  // Final Server
  struct sockaddr_storage final_server_addr;
  socklen_t final_server_len;
  // int server_domain;   
  int final_server_fd;

  // Estado del Socket:
  struct state_machine stm;

  // estado del socket cliente
  union client_data {
    hello_data hello;
    auth_data auth;
    request_data request;
    resolv_data resolv;
    copy_data copy;
  } client_data;

  // estados del socket final server
  union server_data {
    // resolve_st resolve;
    connecting_data connect;
    copy_data copy;
  } server_data;

  // buffer de escritura
  uint8_t raw_buff_a[BUFFER_SIZE], raw_buff_b[BUFFER_SIZE];
  buffer read_buffer, write_buffer;

  enum socks_response_status status; //enum status

  int toFree;
} socks5;

#endif
