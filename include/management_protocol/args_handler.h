#ifndef ARGS_HANDLER_MANAGER_H
#define ARGS_HANDLER_MANAGER_H

#include <stdlib.h>
#include <string.h>
#include "../args.h"

#define DEFAULT_MNG_PORT 8080
#define DEFAULT_MNG_ADDR_V4 "127.0.0.1"

struct parsers {

} parsers;

struct manage_args {
    bool authorized;

    char* mng_addr;
    unsigned int mng_port;
    uint8_t version;

    struct parsers* parserList;

    char* try_password;

    bool list_flag;
    unsigned int list_option;

    bool get_flag;
    unsigned int get_option;

    bool add_flag;
    char* add_username;
    char* add_password;

    bool delete_flag;

    bool toggle_flag;
    char* toggle_option;
    char* toggle_status;
    int new_buffsize;

} manage_args;

void parse_args(const int argc, char** argv, struct manage_args* mng_args);

#endif