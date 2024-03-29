#ifndef ADMIN_CONNECT_PARSER_H
#define ADMIN_CONNECT_PARSER_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdint.h>
#include "../buffer.h"
#include "../logger.h"



#define MAX_PASS_SIZE 0XFF

enum admin_connect_state {
  admin_connect_version,
  admin_connect_passlen,
  admin_connect_passwd,
  admin_connect_done,
  admin_connect_error,
  admin_connect_error_version,
  admin_connect_error_passlen,
};

struct password {
  uint8_t passlen;
  uint8_t passwd[MAX_PASS_SIZE];
};

typedef struct admin_connect_parser {
  enum admin_connect_state state;
  struct password password;
  uint8_t remaining;
  uint8_t read;
  uint8_t version;
  uint8_t status;
} admin_connect_parser;

void admin_connect_parser_init(struct admin_connect_parser *p);

enum admin_connect_state admin_connect_parser_feed(admin_connect_parser *p, uint8_t b);

enum admin_connect_state admin_connect_consume(buffer *b, admin_connect_parser *p, bool *error);

bool admin_connect_is_done(const enum admin_connect_state state, bool *error);

int admin_connect_marshall(buffer *b, const uint8_t status);

#endif
