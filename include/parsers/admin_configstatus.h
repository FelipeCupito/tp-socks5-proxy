#ifndef ADMIN_CONFIGSTATUS_PARSER_H
#define ADMIN_CONFIGSTATUS_PARSER_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdint.h>
#include "../buffer.h"
#include "../logger.h"



enum admin_configstatus_state {
  admin_configstatus_action,
  admin_configstatus_field,
  admin_configstatus_status,
  admin_configstatus_done,
  admin_configstatus_error,
  admin_configstatus_error_action,
  admin_configstatus_error_field,
  admin_configstatus_error_status,
};

typedef struct admin_configstatus_parser {
  enum admin_configstatus_state state;
  uint8_t action;
  uint8_t field;
  uint8_t status;
  uint8_t config_status;
} admin_configstatus_parser;

void admin_configstatus_parser_init(struct admin_configstatus_parser *p);

enum admin_configstatus_state admin_configstatus_parser_feed(admin_configstatus_parser *p, uint8_t b);

enum admin_configstatus_state admin_configstatus_consume(buffer *b, admin_configstatus_parser *p, bool *error);

bool admin_configstatus_is_done(const enum admin_configstatus_state state, bool *error);

int admin_configstatus_marshall(buffer *b, const uint8_t status);

#endif