#ifndef POP3_SNIFFER_H
#define POP3_SNIFFER_H

#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <sys/socket.h> // socket
#include <sys/types.h> 
#include <errno.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

#include "buffer.h"
#include "logger.h"
#include "listgeneric.h"
#include "socks5.h"

#define MAX_USR_PASS_SIZE 0XFF

static const char * OK = "+OK";
static const char * USER = "USER ";
static const char * PASS = "PASS ";
static const char * ERR = "-ERR";

enum pop3_sniffer_state {
  pop3_sniffer_initial = 0,
  pop3_sniffer_user,
  pop3_sniffer_read_user,
  pop3_sniffer_password,
  pop3_sniffer_read_passsword,
  pop3_sniffer_check_password, // chequea la respuesta
  pop3_sniffer_ok,
  pop3_sniffer_err,
};


listADT list;

typedef struct pop3_sniffer {
  enum pop3_sniffer_state state;
  char username[MAX_USR_PASS_SIZE];
  char passwd[MAX_USR_PASS_SIZE];
  uint8_t read;
  uint8_t remaining;
  uint8_t check_read;
  uint8_t check_remaining;
} pop3_sniffer;

typedef struct sniff_info {
  struct sockaddr_storage *addr;
  socklen_t *server_len;
  char *proxy_username;
  char *user;
  char *passwd;
} sniff_info;

void pop3_sniffer_init(struct pop3_sniffer* s);

enum pop3_sniffer_state pop3_sniffer_feed(struct pop3_sniffer* s, uint8_t b);

enum pop3_sniffer_state pop3_sniffer_consume(buffer *buff, struct pop3_sniffer *s, struct socks5 *socks5);

bool pop3_is_done(struct pop3_sniffer *s);

bool pop3_is_parsing(struct pop3_sniffer *s);

void pop3_sniffer_init_list();

void freeSniffer();

void pop3sniff(uint8_t *ptr, ssize_t size, struct socks5 *socks5, struct pop3_sniffer *s, buffer *buff);

struct sniff_info* getNext();

#endif