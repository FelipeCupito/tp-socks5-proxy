#include "../../include/parsers/hello.h";

/*
struct hello_parser {
  uint8_t version;
  uint8_t NMETHODS; 
  uint8_t METHODS;
  uint8_t non_auth; // ?
  uint8_t * auth;

  unsigned int bytes_to_read;
  hello_state state;
}
*/

void hello_parser_init (hello_parser p) {
  memset(p, 0, sizeof(struct hello_parser));
  p->auth = NULL;
}

enum hello_state hello_read_next(hello_parser p, const uint8_t b) {
  switch(p->state) {
    case VERSION:
      if (b == VERSION_FIELD) {
        p -> state = AUTH;
      } else {
        p -> state = ERROR_INV_VERSION;
      }
      break;
    case NAUTH:
      if (b == 0) {
        p->state = ERROR_INV_NAUTH;
      }
      p->nauth = b; // ?
      p->auth = malloc(b); // ?
      p->bytes_to_read = b; // ?
      p->state = HELLO_AUTH; // ?
      // completar
      break;
    case AUTH:
      if (p->bytes_to_read) {
        p->auth[p->nauth - p->bytes_to_read] = b;
        p->bytes_to_read--;
        if (p->bytes_to_read == 0) {
            p->state = DONE;
          }
      }
    case DONE:   
      break;
    case ERROR_INV_VERSION:
      break;
    case ERROR_INV_NAUTH:
      break;
    case ERROR_UNASSIGNED_METHOD:
      break;
    case ERROR_UNSUPPORTED_METHOD:
      break;
  }

  return p -> state;
}

enum hello_state hello_consume(buffer * b, hello_parser p, bool *errored) {
  hello_state state = p -> state;
  
  while (buffer_can_read(b) && !parsing_done(p, errored)) {
    const uint8_t character = buffer_read(b);
    state = hello_read_next(p, c);
  }

  return state;
}
