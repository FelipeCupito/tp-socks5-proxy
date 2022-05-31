#ifndef ARGS_H_kFlmYm1tW9p5npzDr2opQJ9jM8
#define ARGS_H_kFlmYm1tW9p5npzDr2opQJ9jM8

#include <stdbool.h>
#include <errno.h>
#include <getopt.h>
#include <limits.h>  /* LONG_MIN et al */
#include <stdio.h>   /* for printf */
#include <stdlib.h>  /* for exit */
#include <string.h>  /* memset */

#include "../include/logger.h"

#define MAX_USERS 10
#define DEFAULT_

struct users {
    char *name;
    char *pass;
};

struct socks5args {
    char           *socks_addr; //
    unsigned short  socks_port;

    char *          mng_addr; 
    unsigned short  mng_port;

    bool            disectors_enabled;

    struct users    users[MAX_USERS];
};

/**
 * Interpreta la linea de comandos (argc, argv) llenando
 * args con defaults o la seleccion humana. Puede cortar
 * la ejecución.
 */
void 
parse_args(const int argc,  char **argv, struct socks5args *args);

#endif

