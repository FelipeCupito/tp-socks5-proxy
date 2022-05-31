/**
 * main.c - servidor proxy socks concurrente
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
#include "../include/ipversion.h"
//#include "socks5.h"
//#include "socks5nio.h"

static bool done = false;

static void sigterm_handler(const int signal) {
  printf("signal %d, cleaning up and exiting\n", signal);
  done = true;
}

// lee los argumetos
int main(const int argc, char **argv) {

  struct socks5args arguments;
  parse_args(argc, argv, &arguments);
  // no tenemos nada que leer de stdin
  close(0);
  // close(1);

  const char *err_msg = NULL;
  selector_status ss = SELECTOR_SUCCESS;
  // fd_selector selector = NULL;

  int ip_v = ip_version(arguments.socks_addr);
  int server;
  // IP V4
  if (ip_v == 4) {
    printf("4");
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;

    inet_pton(AF_INET, arguments.socks_addr, &addr.sin_addr);
    // addr.sin_addr.s_addr = htonl(*arguments.socks_addr);
    addr.sin_port = htons(arguments.socks_port);

    server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (server < 0) {
      err_msg = "unable to create socket";
      goto finally;
    }

    fprintf(stdout, "Listening on TCP port %d\n", arguments.socks_port);

    // man 7 ip. no importa reportar nada si falla.
    setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));

    if (bind(server, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
      err_msg = "unable to bind socket_v4";
      goto finally;
    }

    if (listen(server, 20) < 0) {
      err_msg = "unable to listen";
      goto finally;
    }
  }
  // IP v6
  else {
    if (ip_v == 6) {
      printf("6");
      struct sockaddr_in6 addr6;
      memset(&addr6, 0, sizeof(addr6));
      addr6.sin6_family = AF_INET6;
      inet_pton(AF_INET6, arguments.socks_addr, &addr6.sin6_addr);
      //addr.sin_addr.s_addr = htonl(*arguments.socks_addr);
      addr6.sin6_port = htons(arguments.socks_port);

      server = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);

      if (server < 0) {
        err_msg = "unable to create socket_v6";
        goto finally;
      }

      fprintf(stdout, "Listening on TCP port %d\n", arguments.socks_port);

      // man 7 ip. no importa reportar nada si falla.
      setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));

      if (bind(server, (struct sockaddr *)&addr6, sizeof(addr6)) < 0) {
        err_msg = "unable to bind socket_v6";
        goto finally;
      }

      if (listen(server, 20) < 0) {
        err_msg = "unable to listen";
        goto finally;
      }
    }
  } 

  // registrar sigterm es Ãºtil para terminar el programa normalmente.
  // esto ayuda mucho en herramientas como valgrind.
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
