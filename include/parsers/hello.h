#ifndef HELLO_PARSER_H
#define HELLO_PARSER_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdint.h>

#include "buffer.h"

static const uint8_t METHOD_NO_AUTH_REQUIRED = 0X00;
static const uint8_t METHOD_AUTH_REQUIRED = 0x02;
static const uint8_t METHOD_NO_ACCEPTABLE_METHODS = 0XFF;

enum hello_state {
  hello_version,
  // Estamos leyendo la cantidad de metodos
  hello_nmethods,
  // Estamos leyendo los metodos
  hello_methods,
  hello_done,
  hello_error_unsupported_version,
};

struct hello_parser {
  // invocado cada vez que se presenta un nuevo metodo
  // el usuario me pasa una funcion y a esa funcion le paso el parser y el metodo que lei
  void (*on_auth_method) (struct hello_parser *parser, const uint8_t method);
  // permite al user del parser almacenar sus datos
  void *data;
  enum hello_state state;
  // metodos que faltan leer
  uint8_t remaining;
};

struct hello_parser_init (struct hello_parser *p);
// basico, le damos un byte al parser
enum hello_state hello_parser_feed (
  struct hello_parser *p, uint8_t b
);

// te doy el buffer, consumi todos los bytes que puedas
enum hello_state hello_consume (buffer *buff, struct hello_parser *p, bool *errored);

bool hello_is_done (const enum hello_state state, bool *errored);

extern const char * hello_error(const struct hello_parser *p);

void hello_parser_close (struct hello_parser *p);

static const uint8_t SOCKS_HELLO_NOAUTH_REQUIRED = 0x00;
static const uint8_t SOCKS_HELLO_AUTH_REQUIRED = 0x02;
// si el METHOD seleccionado es X'FF', ninguno de los metodos listados por el cliente son 
// aceptables, y el cliente debe cerrar la conexion
static const uint8_t SOCKS_HELLO_NO_ACCEPTABLE_METHODS = 0xFF;

// serializa en un buff la respuesta al hello
// retorn la cantidad de bytes ocupados del buffer
// o -1 si no habia espacio suficiente

int hello_marshall (buffer *b, const uint8_t method);

#endif