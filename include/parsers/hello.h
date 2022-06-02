#ifndef HELLO_PARSER_H
#define HELLO_PARSER_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdint.h>

typedef enum hello_state {
  VERSION = 0,
  NAUTH,
  AUTH,
  DONE,
  ERROR_INV_VERSION, // ?
  ERROR_INV_AUTH, // ?
  ERROR_UNASSIGNED_METHOD,
  ERROR_UNSUPPORTED_METHOD
} hello_state;

struct hello_parser {
  uint8_t version;
  uint8_t NMETHODS; 
  uint8_t METHODS;
  uint8_t non_auth; // ?
  uint8_t * auth;

  unsigned int bytes_to_read;
  hello_state state;
}

typedef enum hello_methods {
  NO_AUTH_REQUIRED = 0x00,
  USERPASS = 0x02,
  NO_ACCEPTABLE = 0xff
}

void hello_parser_init (hello_parser p);

enum hello_state hello_read_next (hello_parser p, const uint8_t b);
// Falta el buffer
// enum hello_state hello_consume (buffer * b, hello_parser p, bool *errored);

int parsing_done (hello_parser p, bool *errored);
int is_done (hello_state s, bool *errored);

uint8_t get_nauth (hello_parser p);
const uint8_t * get_auth_types (hello_parser p);

int hello_marshall (buffer *b, const uint8_t method);

void free (hello_parser p);

hello_state get_state (hello_parser p);

#endif