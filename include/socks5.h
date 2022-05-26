
#include <stdio.h>
#include <stdlib.h> // calloc
#include <string.h> // memcpy
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>   // socket
#include <sys/socket.h>  // socket
#include <netinet/in.h>
#include <netinet/tcp.h>

#include "buffer.h"
#include "selector.h"

#define ATTACHMENT(key) ((struct socks5 *) (key)->data)

void socksv5_passive_accept(struct selector_key *key);


