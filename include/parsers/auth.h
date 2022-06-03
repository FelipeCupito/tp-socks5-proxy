#ifndef AUTH_PARSER_H
#define AUTH_PARSER_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdint.h>

#include "../buffer.h"

/*

+----+------+----------+------+----------+
|VER | ULEN |  UNAME   | PLEN |  PASSWD  |
+----+------+----------+------+----------+
| 1  |  1   | 1 to 255 |  1   | 1 to 255 |
+----+------+----------+------+----------+

*/
static const uint8_t AUTH_SUCCESS = 0X00;
// static const uint8_t MAX_USER_PASS_SIZE = 0XFF;
// static const uint8_t METHOD_AUTH_REQUIRED = 0x02;
// static const uint8_t METHOD_NO_ACCEPTABLE_METHODS = 0XFF;

enum auth_type {
  auth_socks,
  auth_mng,
}

enum auth_state {
  auth_version,
  auth_userlen,
  auth_username,
  auth_passlen,
  auth_pass,

  auth_success,

  auth_error_unsupported_version,
  auth_error,
  // Pensar estos dos:
  // auth_error_user,
  // auth_error_pass,
};

struct user {
    uint8_t userlen;
    uint8_t username[MAX_USR_PASS_SIZE];
};

struct pass {
    uint8_t passlen;
    uint8_t passwd[MAX_USR_PASS_SIZE];
};

typedef struct auth_parser {
  enum auth_state state;

  struct user user;
  struct pass pass;

  uint8_t remaining;
  uint8_t read;
} auth_parser;

void auth_parser_init(struct auth_parser *p,enum auth_type type);

enum auth_state auth_parser_feed(auth_parser *p, uint8_t b);

enum auth_state auth_consume(buffer *b, auth_parser *p, bool *error);

bool auth_is_done(const enum auth_state state, bool *error);

int auth_marshal(buffer *b, const uint8_t status, uint8_t version);

#endif