#ifndef ADMIN_ACTION_PARSER_H
#define ADMIN_ACTION_PARSER_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdint.h>
#include "../buffer.h"

static const uint8_t STATUS_OK = 0X00;
static const uint8_t STATUS_ERROR_INVALID_ACTION = 0x01;
static const uint8_t GET = 0X00;
static const uint8_t SET = 0X01;

enum admin_action_state {
  admin_action_reading,
  admin_action_done,
  admin_action_error_invalid_action,
};

typedef struct admin_action_parser {
  enum admin_action_state state;
  uint8_t action;
} admin_action_parser;

void admin_action_parser_init(struct admin_action_parser *p);

enum admin_action_state admin_action_parser_feed(admin_action_parser *p, uint8_t b);

enum admin_action_state admin_action_consume(buffer *b, admin_action_parser *p, bool *error);

bool admin_action_is_done(const enum admin_action_state state, bool *error);

int admin_action_marshal(buffer *b, const uint8_t status, admin_action_parser *p);

#endif