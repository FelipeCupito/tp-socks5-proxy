#include "../../include/parsers/admin_get.h"

void admin_get_parser_init (struct admin_get_parser *p) {
  p -> state = admin_get_action;
}

enum admin_get_state get_option(const uint8_t b, struct admin_get_parser* p) {
  enum admin_get_state next;
  switch (b) {
    case users:
    case passwords:
    case historic_connections:
    case current_connectios:
    case sent_bytes:
    case recv_bytes:
    case buffsize:
    case auth_status:
    case spoofing_status:
      p -> option = b;
      next = admin_get_done;
    break;
    default:
      next = admin_get_error_option;
    break;
  }

  return next;
}

enum admin_get_state get_action(const uint8_t b, struct admin_get_parser* p) {
  enum admin_get_state next = admin_get_error_option;
  if (b == GET_ACTION) {
    next = admin_get_option;
  }

  return next;
}

enum admin_get_state admin_get_parser_feed (admin_get_parser *p, uint8_t b) {
  switch (p -> state) {
    case admin_get_action:
      p -> state = get_action(b,p);
      break;
    case admin_get_option:
      p -> state = get_option(b,p);
      break;
    case admin_get_done:
    case admin_get_error:
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

extern int admin_get_marshall(buffer *b, const uint8_t status, uint8_t *res) {
  size_t n;

  uint8_t *buff = buffer_write_ptr(b, &n);
  if (n < 3) {
    return -1;
  }

  size_t len = n + sizeof(res);
  buff[0] = status;
  buff[1] = sizeof(res);
  memcpy(&buff[2], res, sizeof(*res));
  buffer_write_adv(b, len);
  return len;
}