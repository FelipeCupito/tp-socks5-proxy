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

#include "buffer.h"
#include "metrics.h"

#define MAX_USERS 10

#define MNG_TOKEN "holacomoestas"

#define DEFAULT_SOCKS_BUFFER_SIZE 1024
#define DEFAULT_DISECTORS true

#define MNG_BUFFER_SIZE 4096 // TODO: sacar

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
  bool auth_enabled;
  uint32_t  socks_buffer_size;

  //
  char *mng_token;

  //user config
  struct users users[MAX_USERS];
  int users_size;


} config;

//////////////////////////////////////////////////////////////////////////////////////////////////////////

config* init_config();

//checks
int checkToken(char *token);


//getters
int get_users(buffer *buff_write);

int get_pass(buffer *buff_write);

//devulve 1 si ok, 0 sino
int checkUser(char *user, char *pass);

int get_buff_size(buffer *buff_write);

int get_auth_status(buffer *buff_write);

int get_spoofing_status(buffer *buff_write);

int _get_sent_bytes(buffer *buff_write);

int _get_received_bytes(buffer *buff_write);

int _get_transfered_bytes(buffer *buff_write);

int _get_current_conn(buffer *buff_write);

int _get_histori_conn(buffer *buff_write);

//is
int is_spoofing_enabled();

int is_auth_enabled();

#endif
