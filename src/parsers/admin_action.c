#include "../../include/parsers/admin_action.h"

void admin_action_parser_init (struct admin_action_parser *p) {
  p -> state = admin_action_reading;
}

enum admin_action_state admin_action_parser_feed (admin_action_parser *p, uint8_t b) {
  switch (p -> state){
    case admin_action_reading:
      if (b == GET || b == SET) {
        p -> action = b;
        p -> state = admin_action_done;
      } else {
        p -> state = admin_action_error_invalid_action;
      }
      break;
    case admin_action_done:
      break;
    case admin_action_error_invalid_action:
      break;
    default:
      break;
  }

  return p -> state;
}

bool admin_action_is_done(const enum admin_action_state state, bool *errored ) {
  if (state == admin_action_done || state == admin_action_error_invalid_action) {
    if (state == admin_action_error_invalid_action) {
      if (errored != 0) {
        *errored = true;
      }
    }
    return true;
  } 

  return false;
}

enum admin_action_state admin_action_consume(buffer *buff, struct admin_action_parser *p, bool *errored) {
  enum admin_action_state state = p->state;

  while (buffer_can_read(buff)) {
    const uint8_t c = buffer_read(buff);
    state = admin_action_parser_feed(p, c);
    if (admin_action_is_done(state, errored)) {
      break;
    }
  }

  return state;
}

extern int admin_action_marshall(buffer *b, const uint8_t status, struct admin_action_parser *parser) {
  size_t n;
  uint8_t *buff = buffer_write_ptr(b, &n);

  if (n < 2) {
    return -1;
  }

  buff[0] = status;
  buff[1] = parser -> action;
  buffer_write_adv(b, 1);
  return 1;
}