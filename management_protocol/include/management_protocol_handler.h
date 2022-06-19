#ifndef MANAGEMENT_PROTOCOL_HANDLER_H
#define MANAGEMENT_PROTOCOL_HANDLER_H

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include "args_handler.h"

#define MAX_FIELD_SIZE 256
#define SERVER_ERROR 0xFF

#define GET_MSG_SIZE 3
#define PUT_MSG_SIZE 4
#define EDIT_MSG_SIZE 6
#define DELETE_MSG_SIZE 5
#define CONFIGSTATUS_MSG_SIZE 4
#define CONFIGBUFFSIZE_MGS_SIZE 4

static const uint8_t STATUS_OK                     = 0x00;
static const uint8_t STATUS_ERROR_INVALID_ACTION   = 0x01;
static const uint8_t STATUS_ERROR_INVALID_FIELD    = 0x02;
static const uint8_t STATUS_ERROR_INVALID_STATUS    = 0x03;

static const uint8_t CONFIGSTATUS_ACTION = 0x04;

static const uint8_t CONFIGSTATUS_AUTH_FIELD = 0x03;
static const uint8_t CONFIGSTATUS_SPOOFING_FIELD = 0x04;

char* get_msg[] = {"Success", "Invalid action", "Invalid option"};
char* put_msg[] = {"Success", "Invalid action", "Invalid username length", "Invalid password length"};
char* edit_msg[] = {"Success", "Invalid action", "Invalid field", "Invalid username length", "Invalid attribute", "Invalid value length"};
// TODO: Preguntar que es invalid field en delete
char* delete_msg[] = {"Success", "Invalid action", "Invalid field", "Invalid username length", "Unknown user fail"};
char* configstatus_msg[] = {"Success", "Invalid action", "Invalid field", "Invalid status"};
char* configbuffsize_msg[]= {"Success", "Invalid action", "Invalid buffer size length", "Invalid buffer size"};

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

void login(int fd, struct manage_args* args);
void executeCommands(int fd, struct manage_args* args);

#endif
