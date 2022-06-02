#ifndef HELLO_PARSER_H
#define HELLO_PARSER_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdint.h>

#include "buffer.h"

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
  // uint8_t NMETHODS;  
  // uint8_t METHODS;
  uint8_t non_auth; // TODO ?
  uint8_t * auth;

  unsigned int bytes_to_read;
  hello_state state;
}

static const uint8_t HELLO_NOAUT_REQUIRED = 0x00; // TODO no se si hace falta
static const uint8_t HELLO_USERPASS = 0x02;
static const uint8_t HELLO_NO_ACCEPTABLE_METHODS = 0xFF;

void hello_parser_init (struct hello_parser *p);

enum hello_state hello_parser_feed (hello_parser p, const uint8_t b);
enum hello_state hello_consume (buffer * b, hello_parser p, bool *errored);

int parsing_done (hello_parser p, bool *errored); // podria ser bool o no
int is_done (hello_state s, bool *errored);

uint8_t get_nauth (hello_parser p); // TODO
const uint8_t * get_auth_types (hello_parser p);

int hello_marshall (buffer *b, const uint8_t method);

int hello_parser_close(struct hello_parser *p);
void free_hello_parser(hello_parser p);

hello_state get_state (hello_parser p);

static bool read_hello(const int fd, const uint8_t *method)

#endif