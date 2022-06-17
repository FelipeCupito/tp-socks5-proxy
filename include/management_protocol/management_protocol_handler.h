#ifndef MANAGEMENT_PROTOCOL_HANDLER_H
#define MANAGEMENT_PROTOCOL_HANDLER_H

#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include "args_handler.h"

#define GET_RESPONSE_SIZE 256

// Chequear desp cuando mergeemos branches
// Agregar STATUS_ERROR_IN_PASS
enum connection_status {
    CONN_STATUS_OK = 0x00,
    CONN_STATUS_ERROR_IN_VERSION = 0x01,
    CONN_STATUS_ERROR_IN_PASSLEN = 0x02,
    CONN_STATUS_ERROR_IN_PASS = 0x03,
};

enum get_status {
    GET_STATUS_OK = 0x00,
    GET_STATUS_ERROR_INVALID_ACTION = 0x01,
    GET_STATUS_ERROR_INVALID_OPTION = 0x02,
    GET_STATUS_SERVER_ERROR = 0x03
} get_status;

enum get_options {
    USERS,
    PASSWORDS,
    BUFFERSIZE,
    AUTH_STATUS,
    SPOOFING_STATUS,
    SENT_BYTES,
    RECEIVED_BYTES,
    HISTORIC_CONNECTIONS,
    CONCURRENT_CONNECTIONS,
};

struct get_reply {
    int status;
    char* reply;
} get_reply;

void login(int fd, struct manage_args* args);
void executeCommands(int fd, struct manage_args* args);

#endif