#ifndef ARGS_H_kFlmYm1tW9p5npzDr2opQJ9jM8
#define ARGS_H_kFlmYm1tW9p5npzDr2opQJ9jM8

#include <arpa/inet.h>
#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <unistd.h>
#include <limits.h>  /* LONG_MIN et al */
#include <stdio.h>   /* for printf */
#include <stdlib.h>  /* for exit */
#include <string.h>  /* memset */
#include <sys/socket.h> // socket
#include <sys/types.h>  // socket

#include "../include/logger.h"

//default:
#define MAX_USERS 10
#define SELECT_TIMEOUT 10

//socks proxy
#define DEFAULT_SOCKS_BUFFER_SIZE 1024
#define DEFAULT_SOCKS_ADDRESS_V4 "127.0.0.1"
#define DEFAULT_SOCKS_ADDRESS_V6 "::1"
#define DEFAULT_SOCKS_PORT 1080

//mng server
#define DEFAULT_MNG_ADDRESS_V4 "127.0.0.0"
#define DEFAULT_MNG_ADDRESS_V6 "::"
#define DEFAULT_MNG_PORT 8080


//variables:
struct users {
    char *name;
    char *pass;
};

typedef struct args{
    
    // server
    struct sockaddr_in socksV4;
    bool socksV4_flag;
    struct sockaddr_in6 socksV6;
    bool socksV6_flag; 

    // mng
    struct sockaddr_in mngV4;
    bool mngV4_flag;
    struct sockaddr_in6 mngV6;
    bool mngV6_flag; 

    //server config
    bool disectors_enabled;
    uint16_t socks_buffer_size;
    uint8_t timeout;

    //user config
    struct users users[MAX_USERS];
    int current_users;
    
} args;

/**
 * Interpreta la linea de comandos (argc, argv) llenando
 * args con defaults o la seleccion humana. Puede cortar
 * la ejecución.
 */
void parse_args(const int argc,  char **argv, args* arguments);

#endif

