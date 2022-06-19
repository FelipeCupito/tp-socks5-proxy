#ifndef ADMIN_PUT_PARSER_H
#define ADMIN_PUT_PARSER_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdint.h>
#include "../buffer.h"
#include "../logger.h"



#define MAX_USR_PASS_SIZE 0XFF

enum admin_put_state {
  admin_put_action,
  admin_put_field,
  admin_put_namelen,
  admin_put_name,
  admin_put_passlen,
  admin_put_pass,
  admin_put_error,
  admin_put_error_action,
  admin_put_error_field,
  admin_put_error_namelen,
  admin_put_error_passlen,
  admin_put_done,
};

struct username {
    uint8_t userlen;
    uint8_t username[MAX_USR_PASS_SIZE];
};

struct put_pass {
    uint8_t passlen;
    uint8_t passwd[MAX_USR_PASS_SIZE];
};

typedef struct admin_put_parser {
  enum admin_put_state state;
  uint8_t field;
  uint8_t action;
  struct username user;
  struct put_pass pass;
  uint8_t remaining;
  uint8_t read;
  uint8_t status;
} admin_put_parser;

void admin_put_parser_init(struct admin_put_parser *p);

enum admin_put_state admin_put_parser_feed(admin_put_parser *p, uint8_t b);

enum admin_put_state admin_put_consume(buffer *b, admin_put_parser *p, bool *error);

bool admin_put_is_done(const enum admin_put_state state, bool *error);

int admin_put_marshall(buffer *b, const uint8_t status);

#endif