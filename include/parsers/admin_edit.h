#ifndef ADMIN_EDIT_PARSER_H
#define ADMIN_EDIT_PARSER_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdint.h>
#include "../buffer.h"
#include "../logger.h"



#define MAX_VALUE_SIZE 0XFF

enum attribute {
  username = 0x00,
  password,
};

enum admin_edit_state {
  admin_edit_action,
  admin_edit_field,
  admin_edit_ulen,
  admin_edit_username,
  admin_edit_attribute,
  admin_edit_valuelen,
  admin_edit_value,
  admin_edit_done,
  admin_edit_error,
  admin_edit_error_action,
  admin_edit_error_field,
  admin_edit_error_ulen,
  admin_edit_error_attribute,
  admin_edit_error_valuelen,
};

typedef struct admin_edit_parser {
  enum admin_edit_state state;
  uint8_t ulen;
  uint8_t username[MAX_VALUE_SIZE];
  uint8_t field;
  uint8_t action;
  enum attribute attr;
  uint8_t valuelen;
  uint8_t value[MAX_VALUE_SIZE];
  uint8_t remaining;
  uint8_t read;
  uint8_t status;
} admin_edit_parser;

void admin_edit_parser_init(struct admin_edit_parser *p);

enum admin_edit_state admin_edit_parser_feed(admin_edit_parser *p, uint8_t b);

enum admin_edit_state admin_edit_consume(buffer *b, admin_edit_parser *p, bool *error);

bool admin_edit_is_done(const enum admin_edit_state state, bool *error);

int admin_edit_marshall(buffer *b, const uint8_t status);

#endif