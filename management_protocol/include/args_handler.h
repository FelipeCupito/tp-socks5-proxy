#ifndef ARGS_HANDLER_MANAGER_H
#define ARGS_HANDLER_MANAGER_H

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <limits.h>
#include <errno.h>
#include "../../include/logger.h"

#define DEFAULT_MNG_PORT 8080
#define DEFAULT_MNG_ADDR_V4 "127.0.0.1"

typedef struct manage_args {
    bool authorized;

    char* mng_addr;
    unsigned int mng_port;
    uint8_t version;

    char* try_password;

    bool get_flag;
    char* get_option;

    bool add_flag;
    char* add_username;
    char* add_password;

    bool delete_flag;
    char* delete_username;

    bool edit_flag;
    char* edit_username;
    unsigned int edit_attribute;
    char* edit_value;

    bool toggle_flag;
    char* toggle_option;
    char* toggle_status;

    bool set_flag;
    unsigned int set_size;

} manage_args;

void parse_args_handler(const int argc, char** argv, struct manage_args* mng_args);

#endif