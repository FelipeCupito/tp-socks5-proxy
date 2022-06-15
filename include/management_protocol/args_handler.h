#ifndef ARGS_HANDLER_MANAGER_H
#define ARGS_HANDLER_MANAGER_H

#include "../args.h"

#define DEFAULT_MNG_PORT 8080
#define DEFAULT_MNG_ADDR_V4 "127.0.0.1"



void parse_args(const int argc, char** argv, manage_args* mng_args);

#endif