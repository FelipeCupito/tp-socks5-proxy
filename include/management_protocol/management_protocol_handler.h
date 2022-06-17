#ifndef MANAGEMENT_PROTOCOL_HANDLER_H
#define MANAGEMENT_PROTOCOL_HANDLER_H

#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

void login(int fd, char* password);

#endif