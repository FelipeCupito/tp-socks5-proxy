#include "../../include/parsers/admin_get.h"
#include "../../include/logger.h"

void admin_get_parser_init (struct admin_get_parser *p) {
  p -> state = admin_get_field;
}

enum admin_get_state field(const uint8_t b, struct admin_get_parser* p) {
  enum admin_get_state next;
  switch (b) {
    case users:
    case passwords:
    case connections:
    case bytes:
    case buffsize:
    case auth_status:
    case spoofing_status:
      p -> field = b;
      next = admin_get_option;
    break;
    default:
      next = admin_get_error_field;
    break;
  }

  return next;
}

enum admin_get_state option(const uint8_t b, struct admin_get_parser* p) {
  enum admin_get_state next;
  uint8_t f = p -> field;

  switch (b) {
    case none:
      if (f == users || f == passwords || f == buffsize || f == auth_status || f == spoofing_status) {
        p -> option = b;
        next = admin_get_done;
      } else {
        next = admin_get_error_option;
      }
      break;
    case sent_bytes:
    case received_bytes:
      if (f == bytes) {
        p -> option = b;
        next = admin_get_done;
      } else {
        next = admin_get_error_option;
      }
      break;
    case historic_connections:
    case current_connections:
      if (f == connections) {
        p -> option = b;
        next = admin_get_done;
      } else {
        next = admin_get_error_option;
      }
      break;
    default:
      next = admin_get_error;
      break;
  }

  return next;
}

enum admin_get_state admin_get_parser_feed (admin_get_parser *p, uint8_t b) {
  switch (p -> state) {
    case admin_get_field:
      p -> state = field(b,p);
      break;
    case admin_get_option:
      p -> state = option(b,p);
    case admin_get_done:
    case admin_get_error:
    case admin_get_error_field:
    case admin_get_error_option:
      break;
    default:
      log_print(FATAL, "Invalid state %d.\n", p->state);
      break;
  }

  return p -> state;
}

bool admin_get_is_done (const enum admin_get_state state, bool *error){
  bool done = false;

  switch (state) {
    case admin_get_error:
    case admin_get_error_field:
    case admin_get_error_option:
      if (error != 0) {
        *error = true;
      }
    case admin_get_done:
      done = true;
      break;
    default:
      break;
  }

  return done;
}


enum admin_get_state admin_get_consume(buffer *buff, struct admin_get_parser *p, bool *errored) {
  enum admin_get_state state = p->state;

  while (buffer_can_read(buff)) {
    const uint8_t c = buffer_read(buff);
    state = admin_get_parser_feed(p, c);
    if (admin_get_is_done(state, errored)) {
      break;
    }
  }

  return state;
}

extern int admin_get_marshall(buffer *b, const uint8_t status, struct admin_get_parser *parser, uint8_t *res) {
  size_t n;

  uint8_t *buff = buffer_write_ptr(b, &n);
  if (res != NULL && n < 5) {
    return -1;
  }

  size_t len = n + sizeof(res);
  buff[0] = status;
  buff[1] = parser -> field;
  buff[2] = parser -> option;
  buff[3] = sizeof(res);
  memcpy(&buff[4], res, sizeof(*res));
  buffer_write_adv(b, len);
  return len;
}