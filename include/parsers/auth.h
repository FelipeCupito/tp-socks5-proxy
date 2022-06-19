#ifndef AUTH_PARSER_H
#define AUTH_PARSER_H

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "../buffer.h"
#include "../logger.h"

/*
+----+------+----------+------+----------+
|VER | ULEN |  UNAME   | PLEN |  PASSWD  |
+----+------+----------+------+----------+
| 1  |  1   | 1 to 255 |  1   | 1 to 255 |
+----+------+----------+------+----------+
*/

static const uint8_t AUTH_SUCCESS = 0X00;
static const uint8_t AUTH_FAIL = 0X01;
#define MAX_USR_PASS_SIZE 0XFF

enum auth_state {
  auth_version,
  auth_userlen,
  auth_username,
  auth_passlen,
  auth_pass,

  auth_done,

  auth_error_unsupported_version,
  auth_error,
  // TODO Pensar estos dos:
  auth_error_user_len,
  auth_error_pass_len,
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

  uint8_t version; // necesario?
} auth_parser;

void auth_parser_init(struct auth_parser *p);

enum auth_state auth_parser_feed(auth_parser *p, uint8_t b);

enum auth_state auth_consume(buffer *b, auth_parser *p, bool *error);

bool auth_is_done(const enum auth_state state, bool *error);

int auth_marshall(buffer *b, const uint8_t status);

#endif