#ifndef MANAGEMENT_PROTOCOL_HANDLER_H
#define MANAGEMENT_PROTOCOL_HANDLER_H

#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include "args_handler.h"
#include "../parsers/admin_get.h"
#include "../parsers/admin_configstatus.h"

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

enum set_status {
    SET_STATUS_OK = 0x00,
    SET_STATUS_INVALID_ACTION = 0x01,
    SET_STATUS_INVALID_SIZE_LEN = 0x02,
    SET_STATUD_INVALID_SIZE = 0x03
} set_status;

enum toggle_status {
    TOGGLE_STATUS_OK = 0x00,
    TOGGLE_STATUS_INVALID_ACTION = 0x01,
    TOGGLE_STATUS_INVALID_FIELD = 0x02,
    TOGGLE_STATUS_INVALID_STATUS = 0x03,
} toggle_status;

enum list_options {
    USERS,
    PASSWORDS,
    BUFFERSIZE,
    AUTH_STATUS,
    SPOOFING_STATUS,
} list_options;

enum get_options {
    SENT_BYTES,
    RECEIVED_BYTES,
    HISTORIC_CONNECTIONS,
    CONCURRENT_CONNECTIONS,
} get_options;

struct get_reply {
    int status;
    char* reply;
} get_reply;

void login(int fd, struct manage_args* args);
void executeCommands(int fd, struct manage_args* args);

#endif