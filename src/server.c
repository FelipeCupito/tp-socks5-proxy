/**
 * server.c - servidor proxy socks concurrente
 *
 * Interpreta los argumentos de línea de comandos, y monta un socket
 * pasivo.
 *
 * Todas las conexiones entrantes se manejarán en éste hilo.
 *
 * Se descargará en otro hilos las operaciones bloqueantes (resolución de
 * DNS utilizando getaddrinfo), pero toda esa complejidad está oculta en
 * el selector.
 */
#include <arpa/inet.h>
#include <errno.h>
#include <limits.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h> // socket
#include <sys/types.h>  // socket
#include <unistd.h>

#include "../include/args.h"
#include "../include/client_handler.h"
#include "../include/selector.h"

#define SOCKS_BACKLOG 100 // la longitud máxima de la cola de conexiones pendientes
#define MAX_FD 1024   //cantidad maxima de fd de aceptados por el selector
#define INET_ADDRSTRLEN 16
#define INET6_ADDRSTRLEN 46

//funciones privadas
char* init_server_v4(int *fd, struct sockaddr_in addr );
char* init_server_v6(int *fd, struct sockaddr_in6 addr );
static void sigterm_handler(const int signal);

//variables
static bool done = false;

int passive_fds[] = {-1, -1, -1, -1};
int passive_fds_size = 4;
typedef enum passive_socket{
  SOCKS_V4=0,
  SOCKS_V6,
  MNG_V4,
  MNG_V6,
}passive_socket;


int main(const int argc, char **argv) {


  //obtenemos la config inicial y se guarda en cong
  args arguments; 
  parse_args(argc, argv, &arguments);

  //no se usa ni stdout ni stdin
  close(0);
  close(1);

  //se guarda el mensaje de error
  char *err_msg = NULL;

  //variables del selector
  selector_status ss = SELECTOR_SUCCESS;
  fd_selector selector = NULL;
  
  //creo el socket pasivo
  if(arguments.socksV4_flag){
    err_msg = init_server_v4( &passive_fds[SOCKS_V4], arguments.socksV4);
    if( err_msg != NULL )
      goto finally;
  }
  if(arguments.socksV6_flag){
    err_msg = init_server_v6( &passive_fds[SOCKS_V6], arguments.socksV6);
    if( err_msg != NULL )
      goto finally;
  } 
  if(arguments.mngV4_flag){
    err_msg = init_server_v4( &passive_fds[MNG_V4], arguments.mngV4);
    if( err_msg != NULL )
      goto finally;
  } 
  if(arguments.mngV6_flag){
    err_msg = init_server_v6( &passive_fds[MNG_V6], arguments.mngV6);
    if( err_msg != NULL )
      goto finally;
  }
  
  int server = passive_fds[SOCKS_V4];

  // registrar sigterm es Ãºtil para terminar el programa normalmente.
  signal(SIGTERM, sigterm_handler);
  signal(SIGINT, sigterm_handler);
  
  if (selector_fd_set_nio(server) == -1) {
    err_msg = "getting server socket flags";
    goto finally;
  }

  const struct selector_init conf = {
      .signal = SIGALRM,
      .select_timeout =
          {
              .tv_sec = 10,
              .tv_nsec = 0,
          },
  };

  if (0 != selector_init(&conf)) {
    err_msg = "initializing selector";
    goto finally;
  }
  
  selector = selector_new(MAX_FD);
  if (selector == NULL) {
    err_msg = "unable to create selector";
    goto finally;
  }

  const struct fd_handler socksv5 = {
      .handle_read = client_passive_accept,
      .handle_write = NULL,
      .handle_close = NULL,
  };

  //creo el select
  ss = selector_register(selector, server, &socksv5, OP_READ, NULL);
  if (ss != SELECTOR_SUCCESS) {
    err_msg = "registering fd";
    goto finally;
  }
  for (; !done;) {
    err_msg = NULL;
    ss = selector_select(selector);
    if (ss != SELECTOR_SUCCESS) {
      err_msg = "serving";
      goto finally;
    }
  }
  if (err_msg == NULL) {
    err_msg = "closing";
  }

  int ret = 0;
finally:
  if (ss != SELECTOR_SUCCESS) {
    log_print(LOG_ERROR, "%s: %s\n", 
    (err_msg == NULL) ? "" : err_msg,
    ss == SELECTOR_IO ? strerror(errno) : selector_error(ss));
    ret = 2;
  } else if (err_msg) {
    log_print(LOG_ERROR, "%s", err_msg);
    ret = 1;
  }
  if (selector != NULL) {
    selector_destroy(selector);
  }
  selector_close();

  for (int i = 0; i < passive_fds_size; i++){
    if( passive_fds[i]!= -1)
      close(passive_fds[i]);
  }
  
  return ret;
}


////////////////////////////////////////////////////////////////////////////////////////////
//PRIVADAS
////////////////////////////////////////////////////////////////////////////////////////////

// creacion del socket para IPv4
char* init_server_v4(int *fd, struct sockaddr_in addr ){
  
  *fd = socket(addr.sin_family, SOCK_STREAM, IPPROTO_TCP);
  if (*fd < 0) {
    return "unable to create socket";
  }

  setsockopt(*fd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));

  if (bind(*fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
      return "unable to bind socket_v4";
    } 

  if (listen(*fd, SOCKS_BACKLOG) < 0) {
    return "unable to listen";
  }

  char ip[INET_ADDRSTRLEN];
  inet_ntop(addr.sin_family, &addr.sin_addr, ip, INET_ADDRSTRLEN);
  log_print(INFO,"socketV4: %d, Listening on %s port: %d",*fd, ip, ntohs(addr.sin_port));

  return NULL;
}

// creacion del socket IPv6
char* init_server_v6(int *fd, struct sockaddr_in6 addr ){
  
  *fd = socket(addr.sin6_family, SOCK_STREAM, IPPROTO_TCP);
  if (*fd < 0) {
    return "unable to create socket";
  }
  
  setsockopt(*fd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));

  if (bind(*fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
      return "unable to bind socket_v6";
    } 

  if (listen(*fd, SOCKS_BACKLOG) < 0) {
    return "unable to listen";
  }

  char ip[INET6_ADDRSTRLEN];
  inet_ntop(addr.sin6_family, &addr.sin6_addr, ip, INET6_ADDRSTRLEN);
  log_print(INFO,"socketV6: %d, Listening on %s port: %d",*fd, ip, ntohs(addr.sin6_port));

  return NULL;
}

static void sigterm_handler(const int signal) {
  log_print(INFO, "signal %d, cleaning up and exiting ", signal);
  done = true;
}
