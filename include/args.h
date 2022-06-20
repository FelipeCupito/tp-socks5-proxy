#ifndef ARGS_H_kFlmYm1tW9p5npzDr2opQJ9jM8
#define ARGS_H_kFlmYm1tW9p5npzDr2opQJ9jM8

#include <arpa/inet.h>
#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <unistd.h>
#include <limits.h>  /* LONG_MIN et al */
#include <stdio.h>   /* for printf */
#include <stdlib.h>  /* for exit */
#include <string.h>  /* memset */
#include <sys/socket.h> // socket
#include <sys/types.h>  // socket

#include "logger.h"
#include "server_config.h"


//socks proxy
#define DEFAULT_SOCKS_ADDRESS_V4 "127.0.0.1"
#define DEFAULT_SOCKS_ADDRESS_V6 "::1"
#define DEFAULT_SOCKS_PORT 1080

//mng server
#define DEFAULT_MNG_ADDRESS_V4 "127.0.0.1"
#define DEFAULT_MNG_ADDRESS_V6 "::1"
#define DEFAULT_MNG_PORT 8080


/**
 * Interpreta la linea de comandos (argc, argv) llenando
 * args con defaults o la seleccion humana. Puede cortar
 * la ejecución.
 */
void parse_args(const int argc,  char **argv, config* arguments);

#endif

