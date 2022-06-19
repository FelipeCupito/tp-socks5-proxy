#ifndef SERVER_CONFIG_H
#define SERVER_CONFIG_H

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


#define MAX_USERS 10
#define SELECT_TIMEOUT 10
#define DEFAULT_SOCKS_BUFFER_SIZE 1024
#define DEFAULT_DISECTORS true

//variables:
struct users {
  char *name;
  char *pass;
};

typedef struct config{

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

  //user config
  struct users users[MAX_USERS];
  int users_size;

} config;

//////////////////////////////////////////////////////////////////////////////////////////////////////////

config* init_config();

//devulve 1 si ok, 0 sino
int checkUser(char *user, char *pass);

int is_auth_enabled();

#endif