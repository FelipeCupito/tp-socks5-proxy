#ifndef SOCKS5_CONNECTION_H
#define SOCKS5_CONNECTION_H

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
#include "stm.h"
#include "selector.h"
#include "logger.h"
#include "socks5.h"
#include "client_handler.h"

#define SIN_ADDR(sin) (void *)&(((struct sockaddr_in *)sin)->sin_addr)
#define SIN_PORT(sin) (void *)&(((struct sockaddr_in *)sin)->sin_port)

#define SIN_ADDR6(sin) (void *)&(((struct sockaddr_in6*)sin)->sin6_addr)
#define SIN_PORT6(sin) (void *)&(((struct sockaddr_in6*)sin)->sin6_port)


void connecting_init(const unsigned state, struct selector_key *key);
void connecting_close(const unsigned state, struct selector_key *key);
unsigned connecting_read(struct selector_key *key);
unsigned connecting_write(struct selector_key *key);







#endif
