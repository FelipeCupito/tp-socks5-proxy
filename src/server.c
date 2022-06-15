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
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h> // socket
#include <sys/types.h>  // socket
#include <unistd.h>

#include "../include/args.h"
#include "../include/selector.h"
#include "../include/socks5.h"
#include "../include/logger.h"

#define SOCKS_BACKLOG 100               // la longitud máxima de la cola de conexiones pendientes
#define MAX_FD 1024 // cantidad maxima de fd de aceptados por el selector
#define INET_ADDRSTRLEN 16
#define INET6_ADDRSTRLEN 46

// funciones privadas
char *init_server_v4(int *fd, struct sockaddr_in addr);
char *init_server_v6(int *fd, struct sockaddr_in6 addr);
static void sigterm_handler(const int signal);

// variables
static bool done = false;
int passive_fds[] = {-1, -1, -1, -1};
int passive_fds_size = 4;
typedef enum passive_socket {
  SOCKS_V4 = 0,
  SOCKS_V6,
  MNG_V4,
  MNG_V6,
} passive_socket;


int main(const int argc, char **argv) {

  char *err_msg = NULL;                   // se guarda el mensaje de error
  selector_status ss = SELECTOR_SUCCESS;  // status del selector
  fd_selector selector = NULL;            // selector
  args arguments;                         // argumentos del programa

  //guardo los argumntos 
  parse_args(argc, argv, &arguments);

  // cierro stdin y stdout
  close(0);
  close(1);

  //  creo los todo los socket pasivos
  if (arguments.socksV4_flag) {
    err_msg = init_server_v4(&passive_fds[SOCKS_V4], arguments.socksV4);
    if (err_msg != NULL)
      goto finally;
  }
  if (arguments.socksV6_flag) {
    err_msg = init_server_v6(&passive_fds[SOCKS_V6], arguments.socksV6);
    if (err_msg != NULL)
      goto finally;
  }
  if (arguments.mngV4_flag) {
    err_msg = init_server_v4(&passive_fds[MNG_V4], arguments.mngV4);
    if (err_msg != NULL)
      goto finally;
  }
  if (arguments.mngV6_flag) {
    err_msg = init_server_v6(&passive_fds[MNG_V6], arguments.mngV6);
    if (err_msg != NULL)
      goto finally;
  }

  // registrar sigterm es Ãºtil para terminar el programa normalmente.
  signal(SIGTERM, sigterm_handler);
  signal(SIGINT, sigterm_handler);

  // creo el selector
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

  // registro todos los socket pasivos
  for (int fd = 0; fd < passive_fds_size; fd++) {
    if (passive_fds[fd] == -1)
      continue;

    if (selector_fd_set_nio(passive_fds[fd]) == -1) {
      err_msg = "getting server socket flags";
      goto finally;
    }

    if (fd == SOCKS_V4 || fd == SOCKS_V6) {
      ss = selector_register(selector, passive_fds[fd], &socks5_passive_handler,OP_READ, NULL);
    } else {
      // TODO: sacar el &socks5_passive_handler y poner&mng_passive_handler
      ss = selector_register(selector, passive_fds[fd], &socks5_passive_handler, OP_READ, NULL);
    }
    if (ss != SELECTOR_SUCCESS) {
      err_msg = "registering fd";
      goto finally;
    }
  }

  // loop
  for (; !done;) {
    err_msg = NULL;
    ss = selector_select(selector);
    if (ss != SELECTOR_SUCCESS) {
      err_msg = "serving";
      goto finally;
    }
  }

  log_print(INFO, "END");
  int ret = 0;

finally:
  if (ss != SELECTOR_SUCCESS) {
    log_print(LOG_ERROR, "%s: %s\n", (err_msg == NULL) ? "" : err_msg,
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

  for (int i = 0; i < passive_fds_size; i++) {
    if (passive_fds[i] != -1)
      close(passive_fds[i]);
  }

  return ret;
}

////////////////////////////////////////////////////////////////////////////////////////////
// PRIVADAS
////////////////////////////////////////////////////////////////////////////////////////////

// creacion del socket para IPv4
char *init_server_v4(int *fd, struct sockaddr_in addr) {

  *fd = socket(addr.sin_family, SOCK_STREAM, IPPROTO_TCP);
  if (*fd < 0) {
    return "unable to create socket";
  }

  setsockopt(*fd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));

  if (bind(*fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    return "unable to bind socket_v4";
  }

  if (listen(*fd, SOCKS_BACKLOG) < 0) {
    return "unable to listen";
  }

  char ip[INET_ADDRSTRLEN];
  inet_ntop(addr.sin_family, &addr.sin_addr, ip, INET_ADDRSTRLEN);
  log_print(INFO, "socketV4: %d, Listening on %s port: %d", *fd, ip,
            ntohs(addr.sin_port));

  return NULL;
}

// creacion del socket IPv6
char *init_server_v6(int *fd, struct sockaddr_in6 addr) {

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
  log_print(INFO, "socketV6: %d, Listening on %s port: %d", *fd, ip,
            ntohs(addr.sin6_port));

  return NULL;
}

static void sigterm_handler(const int signal) {
  log_print(INFO, "signal %d, cleaning up and exiting ", signal);
  done = true;
}
