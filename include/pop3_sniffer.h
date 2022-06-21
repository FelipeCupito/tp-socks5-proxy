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
#include "server_config.h"

#define MAX_USR_PASS_SIZE 0XFF
#define MAX_BUFF_POP3_SIZE 4096

enum pop3_sniffer_state {
  pop3_sniffer_initial = 0,
  pop3_sniffer_user,
  pop3_sniffer_read_user,
  pop3_sniffer_check_user,
  pop3_sniffer_password,  
  pop3_sniffer_read_passsword,
  pop3_sniffer_check_password, // chequea la respuesta
  pop3_sniffer_ok,
  pop3_sniffer_err,
};




typedef struct pop3_sniffer {
  enum pop3_sniffer_state state;
  char username[MAX_USR_PASS_SIZE];
  char passwd[MAX_USR_PASS_SIZE];
  uint8_t read;
  uint8_t remaining;
  uint8_t check_read;
  uint8_t check_remaining;
  buffer buffer;
  uint8_t raw_buff[MAX_BUFF_POP3_SIZE];
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

//socks5 de tipo struct socks5 si o si
enum pop3_sniffer_state pop3_sniffer_consume(struct pop3_sniffer *s, void *socks5);

bool pop3_is_done(struct pop3_sniffer *s);

bool pop3_is_parsing(struct pop3_sniffer *s);

void pop3_sniffer_init_list();

void freeSniffer();

//socks5 de tipo struct socks5 si o si
void pop3sniff(uint8_t *ptr, ssize_t size, void *socks5);

void begin_Sniffer_List();

int sniffer_hast_next();

struct sniff_info* sniffer_get_next();

#endif
