#ifndef SOCKS5_H
#define SOCKS5_H

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

#include "socks5_connection.h"
#include "socks5_copy.h"

#define MAX_IPS 10
#define IP_V4_ADDR_SIZE 4
#define IP_V6_ADDR_SIZE 16
#define PORT_SIZE 2
#define BUFFER_SIZE 4096 // TODO: sacar

#define ATTACHMENT(key) ((struct socks5 *)(key)->data)

// funciones:
struct socks5 *socks5_new(const int client);

/////////////////////////////////////////////////////////////////////////
// Estados posibles de cada estado de socks5
/////////////////////////////////////////////////////////////////////////
enum socks_state {
  HELLO_READ,
  // creo un nuevo socket y espero conecion
  CONNECTING,
  // copi los datos
  COPY,
  // cierro los sockets
  DONE,
  // error
  ERROR,
};
/////////////////////////////////////////////////////////////////////////
// Estados posibles de cada estado de socks5
/////////////////////////////////////////////////////////////////////////

typedef enum addr_type{
    IPv4 = 0x01,
    DOMAINNAME = 0x03,
    IPv6 = 0x04,
} addr_type;

/////////////////////////////////////////////////////////////////////////
// Estados posibles de cada estado de socks5
/////////////////////////////////////////////////////////////////////////

typedef enum connection_state {
  CONN_INPROGRESS, // EINPROGRESS
  CONN_FAILURE,
  CONN_SUCCESS // EALREADY
} connection_state;

/////////////////////////////////////////////////////////////////////////
// Store de cada estado
/////////////////////////////////////////////////////////////////////////
typedef struct connecting_data {
  buffer *wb;
  unsigned int ip_index;
  int final_server_fd;
  const int client_fd;

} connecting_data;

typedef struct request_data{
  addr_type addrType; //guardo el tipo de conexion
  
  uint8_t ipv4_addrs[MAX_IPS][IP_V4_ADDR_SIZE];
  uint8_t ipv4_size; // cantidad de ipv4, -1 si no hay

  uint8_t ipv6_addrs[MAX_IPS][IP_V6_ADDR_SIZE];
  uint8_t ipv6_size; // cantidad de ipv6, 

  // server port
  uint8_t port[PORT_SIZE];

  // en caso que tengamos que resolver
  uint8_t *resolve_addr;
  uint8_t resolve_addr_len;

}request_data;

typedef struct copy_data {
  /** File descriptor */
  int fd;

  /** buffers para hacer la copia **/
  buffer *rb, *wb;

  /** chequear para saber si cerrar la escritura o la lectura **/
  fd_interest duplex;

  /** Pointer to the structure of the opposing copy state*/
  struct copy_data *other_copy;

} copy_data;

////////////////////////////////////////////////////////////////////////
// struc de cada socks5
////////////////////////////////////////////////////////////////////////
/*
typedef struct final_server {

  struct sockaddr_storage addr;
  socklen_t addr;
  // socklen_t addr_len;//TODO al pedo guardar esto

  

} final_server;
*/
//////////////////////////////////////////////////////////
typedef struct socks5 {

  // cliente:
  struct sockaddr_storage client_addr;
  socklen_t client_addr_len;
  int client_fd; // fd del socket del cliente

  // Final Server
  struct sockaddr_storage final_server_addr;
  socklen_t final_server_len;
  int final_server_fd;

  // Estado del Socket:
  struct state_machine stm;

  // estado del socket cliente
  union client_data{
    // hello_st hello;
    // userpass_st userpass;
    request_data request;
    copy_data copy;
  } client_data;

  // estados del socket final server
  union server_data {
    // resolve_st resolve;
    connecting_data connect;
    copy_data copy;
  } server_data;

  // Nro de referencias a esta instancia de struct socks5
  // Si la cantidad es 1, se puede borrar
  unsigned int references;

  // buffer de escritura
  uint8_t raw_buff_a[BUFFER_SIZE], raw_buff_b[BUFFER_SIZE];
  buffer read_buffer, write_buffer;

  int error;

  // bool disectors_enabled; para mi no hace falta
  // int ss_final_serv; //en caso que usemos dos selects

} socks5;

#endif
