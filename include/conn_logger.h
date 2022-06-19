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
#include <unistd.h>
#include <time.h>

#include "socks5.h"
#include "logger.h"

typedef enum {CONNECTED, DISCONN } STATUS;
//static char* status_str[] = {"[CONNECTED]", "[DISCONNECTED]"};


//data es si o si struct socks5, no se porque no me deja ponerlo 
void log_conn( void* data, STATUS status);

#endif
