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

//variables
static bool done = false;
ptrServerConfig config; 

//funciones privadas
int init_server_v4( const char *err_msg, struct sockaddr_in addr );
int init_server_v6( const char *err_msg, struct sockaddr_in6 addr );
static void sigterm_handler(const int signal);


int main(const int argc, char **argv) {


  //obtenemos la config inicial y se guarda en cong
  config = malloc(sizeof(serverConfig));
  parse_args(argc, argv, config);

  //no se usa ni stdout ni stdin
  close(0);
  close(1);

  //seguarda el mensaje de error
  const char *err_msg = NULL;
  
  //creo el socket pasivo
  int server;
  if(config->is_socks_v4){
    server = init_server_v4(err_msg, config->socks_sockaddr.v4);
  }else{
    server = init_server_v6( err_msg, config->socks_sockaddr.v6);
  }
  
  if(server == -1)
    goto finally;

  // registrar sigterm es Ãºtil para terminar el programa normalmente.
  // esto ayuda mucho en herramientas como valgrind.
  signal(SIGTERM, sigterm_handler);
  signal(SIGINT, sigterm_handler);

  //creo el select
  selector_status ss = SELECTOR_SUCCESS;
  
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

  fd_selector selector = selector_new(1024);
  if (selector == NULL) {
    err_msg = "unable to create selector";
    goto finally;
  }


  const struct fd_handler socksv5 = {
      .handle_read = client_passive_accept,
      .handle_write = NULL,
      .handle_close = NULL,
  };

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
    fprintf(stderr, "%s: %s\n", (err_msg == NULL) ? "" : err_msg,
            ss == SELECTOR_IO ? strerror(errno) : selector_error(ss));
    ret = 2;
  } else if (err_msg) {
    perror(err_msg);
    ret = 1;
  }
  if (selector != NULL) {
    selector_destroy(selector);
  }
  selector_close();

  // socksv5_pool_destroy();

  if (server >= 0) {
    close(server);
  }
  return ret;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////////

// creacion del socket para IPv4
int init_server_v4( const char *err_msg, struct sockaddr_in addr ){
  int server = socket(addr.sin_family, SOCK_STREAM, IPPROTO_TCP);
  if (server < 0) {
    err_msg = "unable to create socket";
    return -1;
  }
  
  log_print(INFO,"Listening on TCP port %d\n", ntohs(addr.sin_port));

  // man 7 ip. no importa reportar nada si falla.
  setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));


  if (bind(server, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
      err_msg = "unable to bind socket_v6";
      return -1;
    } 

  if (listen(server, SOCKS_BACKLOG) < 0) {
    err_msg = "unable to listen";
    return -1;
  }
  return server;
}

// creacion del socket IPv6
int init_server_v6( const char *err_msg, struct sockaddr_in6 addr ){
  int server = socket(addr.sin6_family, SOCK_STREAM, IPPROTO_TCP);
  if (server < 0) {
    err_msg = "unable to create socket";
    return -1;
  }
  
  log_print(INFO,"Listening on TCP port %d\n", ntohs(addr.sin6_port));

  // man 7 ip. no importa reportar nada si falla.
  setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));


  if (bind(server, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
      err_msg = "unable to bind socket_v6";
      return -1;
    } 

  if (listen(server, SOCKS_BACKLOG) < 0) {
    err_msg = "unable to listen";
    return -1;
  }
  return server;
}


static void sigterm_handler(const int signal) {
  printf("signal %d, cleaning up and exiting\n", signal);
  done = true;
}
