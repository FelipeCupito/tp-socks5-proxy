#ifndef ADMIN_GET_PARSER_H
#define ADMIN_GET_PARSER_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdint.h>
#include "../buffer.h"

static const uint8_t STATUS_OK = 0X00;
static const uint8_t STATUS_ERROR_INVALID_FIELD = 0x01;
static const uint8_t STATUS_ERROR_INVALID_OPTION = 0x02;

enum field {
  users = 0x00,
  passwords,
  connections,
  bytes,
  buffsize,
  auth_status,
  spoofing_status,
};

enum option {
  none = 0x00,
  sent_bytes,
  received_bytes,
  historic_connections,
  current_connections,
};

enum admin_get_state {
  admin_get_field,
  admin_get_option,
  admin_get_done,
  admin_get_error,
  admin_get_error_field,
  admin_get_error_option,
};

typedef struct admin_get_parser {
  enum admin_get_state state;
  enum field field;
  enum option option;
  // TODO que hacemos con res?
  // TODO lo separo del parser?
} admin_get_parser;

void admin_get_parser_init(struct admin_get_parser *p);

enum admin_get_state admin_get_parser_feed(admin_get_parser *p, uint8_t b);

enum admin_get_state admin_get_consume(buffer *b, admin_get_parser *p, bool *error);

bool admin_get_is_done(const enum admin_get_state state, bool *error);

int admin_get_marshal(buffer *b, const uint8_t status, admin_get_parser *p, uint8_t *res);

#endif