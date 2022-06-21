#ifndef MANAGEMENT_PROTOCOL_HANDLER_H
#define MANAGEMENT_PROTOCOL_HANDLER_H

#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include "args_handler.h"

#define MAX_FIELD_SIZE 256
#define SERVER_ERROR 0xFF

#define GET_MSG_SIZE 4
#define PUT_MSG_SIZE 5
#define EDIT_MSG_SIZE 8
#define DELETE_MSG_SIZE 6
#define CONFIGSTATUS_MSG_SIZE 5
#define CONFIGBUFFSIZE_MGS_SIZE 5

static const uint8_t STATUS_OK = 0x00;
static const uint8_t STATUS_ERROR_INVALID_ACTION = 0x01;
static const uint8_t STATUS_ERROR_INVALID_FIELD = 0x02;
static const uint8_t STATUS_ERROR_INVALID_STATUS = 0x03;

static const uint8_t CONFIGSTATUS_ACTION = 0x04;

static const uint8_t CONFIGSTATUS_AUTH_FIELD = 0x03;
static const uint8_t CONFIGSTATUS_SPOOFING_FIELD = 0x04;

// Chequear desp cuando mergeemos branches
// Agregar STATUS_ERROR_IN_PASS
enum connection_status {
    CONN_STATUS_OK = 0x00,
    CONN_STATUS_ERROR_IN_VERSION = 0x01,
    CONN_STATUS_ERROR_IN_PASSLEN = 0x02,
    CONN_STATUS_ERROR_IN_PASS = 0x03,
};

void login(int fd, struct manage_args* args);
void execute_commands(int fd, struct manage_args* args);

#endif
