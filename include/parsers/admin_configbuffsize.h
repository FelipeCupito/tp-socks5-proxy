#ifndef ADMIN_CONFIGBUFFSIZE_PARSER_H
#define ADMIN_CONFIGBUFFSIZE_PARSER_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdint.h>
#include "../buffer.h"
#include "../logger.h"



enum admin_configbuff_state {
  admin_configbuff_action,
  admin_configbuff_size,
  admin_configbuff_done,
  admin_configbuff_error,
  admin_configbuff_error_action,
  admin_configbuff_error_sizelen,
  admin_configbuff_error_size,
};

typedef struct admin_configbuff_parser {
  enum admin_configbuff_state state;
  uint8_t size[4];
  uint8_t action;
  uint8_t remaining;
  uint8_t read;
  uint8_t status;
} admin_configbuff_parser;

void admin_configbuff_parser_init(struct admin_configbuff_parser *p);

enum admin_configbuff_state admin_configbuff_parser_feed(admin_configbuff_parser *p, uint8_t b);

enum admin_configbuff_state admin_configbuff_consume(buffer *b, admin_configbuff_parser *p, bool *error);

bool admin_configbuff_is_done(const enum admin_configbuff_state state, bool *error);

int admin_configbuff_marshall(buffer *b, const uint8_t status);

#endif
