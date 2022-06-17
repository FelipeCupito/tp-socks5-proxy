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
    char* mng_addr;
    unsigned int mng_port;
    uint8_t version;

    struct parsers *parserList;

    char* try_password;

    unsigned int list_option;

    unsigned int get_option; 

    char* add_username;
    char* add_password;

    char* toggle_option;
    char* toggle_status;
    
} manage_args;

void parse_args(const int argc, char** argv, struct manage_args* mng_args);

#endif