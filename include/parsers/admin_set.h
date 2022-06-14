#ifndef ADMIN_SET_PARSER_H
#define ADMIN_SET_PARSER_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdint.h>
#include "../buffer.h"
#include "../logger.h"

enum field {
  buffsize = 0x00,
  auth_status,
  spoofing_status,
};

enum admin_set_state {
  admin_set_field,
  admin_set_valuelen,
  admin_set_value,
  admin_set_done,
  admin_set_error,
  admin_set_error_field,
  admin_set_error_valuelen,
  admin_set_error_value,
};

typedef struct admin_set_parser {
  enum admin_set_state state;
  enum field field;
  uint8_t valuelen;
  uint8_t value[0xff];
} admin_set_parser;

void admin_set_parser_init(struct admin_set_parser *p);

enum admin_set_state admin_set_parser_feed(admin_set_parser *p, uint8_t b);

enum admin_set_state admin_set_consume(buffer *b, admin_set_parser *p, bool *error);

bool admin_set_is_done(const enum admin_set_state state, bool *error);

int admin_set_marshal(buffer *b, const uint8_t status, admin_set_parser *p, uint8_t *res);

#endif