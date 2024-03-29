#ifndef ADMIN_DELETE_PARSER_H
#define ADMIN_DELETE_PARSER_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdint.h>
#include "../buffer.h"
#include "../logger.h"

#define MAX_KEY_SIZE 0XFF



enum admin_delete_state {
  admin_delete_action,
  admin_delete_field,
  admin_delete_ulen,
  admin_delete_username,
  admin_delete_done,
  admin_delete_error,
  admin_delete_error_action,
  admin_delete_error_field,
  admin_delete_error_ulen,
};

typedef struct admin_delete_parser {
  enum admin_delete_state state;
  uint8_t ulen;
  uint8_t username[MAX_KEY_SIZE];
  uint8_t field;
  uint8_t action;
  uint8_t read;
  uint8_t remaining;
  uint8_t status;
} admin_delete_parser;

void admin_delete_parser_init(struct admin_delete_parser *p);

enum admin_delete_state admin_delete_parser_feed(admin_delete_parser *p, uint8_t b);

enum admin_delete_state admin_delete_consume(buffer *b, admin_delete_parser *p, bool *error);

bool admin_delete_is_done(const enum admin_delete_state state, bool *error);

int admin_delete_marshall(buffer *b, const uint8_t status);

#endif
