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

#define DEFAULT_SOCKS_BUFFER_SIZE 1024
#define DEFAULT_SOCKS_ADDRESS "127.0.0.1"
#define DEFAULT_SOCKS_PORT 1080

#define DEFAULT_MNG_ADDRESS "127.0.0.1"
#define DEFAULT_MNG_PORT 8080

#define SELECT_TIMEOUT 10

//variables:
struct users {
    char *name;
    char *pass;
};

union IPAddress{
        struct sockaddr_in v4;
        struct sockaddr_in6 v6; 
    }IPAddress;


typedef struct serverConfig{
    //struct sockaddr_storage mng_sockaddr;
    //struct sockaddr_storage socks_sockaddr;
    
    // server
    union IPAddress socks_sockaddr;
    char is_socks_v4;

    // mng
    union IPAddress mng_sockaddr;
    char is_mng_v4;

    //server config
    bool disectors_enabled;
    uint16_t socks_buffer_size;
    uint8_t timeout;

    //user config
    struct users users[MAX_USERS];
    int current_users;
    
} serverConfig;

typedef struct serverConfig * ptrServerConfig;


//funciones:

/**
 * Interpreta la linea de comandos (argc, argv) llenando
 * args con defaults o la seleccion humana. Puede cortar
 * la ejecución.
 */
void parse_args(const int argc,  char **argv, serverConfig *conf);

#endif

