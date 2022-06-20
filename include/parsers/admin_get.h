#ifndef ADMIN_GET_PARSER_H
#define ADMIN_GET_PARSER_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdint.h>
#include "../buffer.h"
#include "../logger.h"



enum option {
  users = 0x00,
  passwords = 0x01,
  buffsize = 0x02,
  auth_status = 0x03,
  spoofing_status = 0x04,
  sent_bytes = 0x05,
  recv_bytes = 0x06,
  historic_connections = 0x07,
  current_connectios = 0x08,
};

enum admin_get_state {
  admin_get_action,
  admin_get_option,
  admin_get_done,
  admin_get_error,
  admin_get_error_action,
  admin_get_error_option,
};

typedef struct admin_get_parser {
  enum admin_get_state state;
  enum option option;
  uint8_t status;
} admin_get_parser;

void admin_get_parser_init(struct admin_get_parser *p);

enum admin_get_state admin_get_parser_feed(admin_get_parser *p, uint8_t b);

enum admin_get_state admin_get_consume(buffer *b, admin_get_parser *p, bool *error);

bool admin_get_is_done(const enum admin_get_state state, bool *error);

int admin_get_marshall(buffer *b, const uint8_t status, char *res, int res_size);

#endif
