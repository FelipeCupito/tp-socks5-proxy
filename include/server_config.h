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
#include "pop3_sniffer.h"

#define MAX_SOCKS_USERS 10
#define MNG_TOKEN "holacomoestas"
#define MAX_CONNECTIONS (508-1) //se reserva una para el mng client

#define DEFAULT_SOCKS_BUFFER_SIZE 1024
#define DEFAULT_DISECTORS true

#define MNG_BUFFER_SIZE 4096
#define MAX_USERNAME_SIZE 10
//variables:
struct users {
  char name[MAX_USERNAME_SIZE];
  char pass[MAX_USERNAME_SIZE];
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
  struct users users[MAX_SOCKS_USERS];
  int users_size;
} config;


//////////////////////////////////////////////////////////////////////////////////////////////////////////

config* init_config();

//checks
int checkToken(char *token);
//devulve 1 si ok, 0 sino
int checkUser(char *user, char *pass);

//delete
int delete_user(char *username);

//edit
int edit_user(char *username, char *new_value, uint8_t attr);

//put
int add_user(char* user, char* pass);

//getters
int get_users(char *res, int res_size);

int get_pop3_pass(char *res, int res_size);

int get_buff_size(char *res, int res_size);

int get_auth_status(char *res, int res_size);

int get_spoofing_status(char *res, int res_size);

int _get_sent_bytes(char *res, int res_size);

int _get_received_bytes(char *res, int res_size);

int _get_transfered_bytes(char *res, int res_size);

int _get_current_conn(char *res, int res_size);

int _get_histori_conn(char *res, int res_size);

uint32_t get_socks_buffer_size();

//configbuff
int set_buff_size(uint8_t* size);
int four_bytes_to_num(uint8_t* src);

//configstatus
int set_auth_status(uint8_t status);
int set_sniff_status(uint8_t status);
void set_status(uint8_t field, uint8_t status);

//is
int is_spoofing_enabled();

int is_auth_enabled();

#endif
