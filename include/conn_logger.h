#ifndef CONN_LOGGER_H
#define CONN_LOGGER_H

#include <errno.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdio.h>
#include <stdlib.h>     // calloc
#include <string.h>     // memcpy
#include <sys/socket.h> // socket
#include <sys/types.h>  // socket
#include <time.h>
#include <unistd.h>

#include "logger.h"
#include "socks5.h"
#include "server_config.h"


// data es si o si struct socks5, no se porque no me deja ponerlo
void log_conn(void *data, enum socks_response_status status);

#endif
