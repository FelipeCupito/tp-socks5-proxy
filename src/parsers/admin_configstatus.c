#include "../../include/parsers/admin_configstatus.h"

void admin_configstatus_parser_init (struct admin_configstatus_parser *p) {
  p -> state = admin_configstatus_action;
}

enum admin_configstatus_state configstatus_action(admin_configstatus_parser *p, uint8_t b) {
  if (b == CONFIGSTATUS_ACTION) {
    p -> action = b;
    p -> state = admin_configstatus_field;
  } else {
    p -> state = admin_configstatus_error_action;
  }

  return p -> state;
}

enum admin_configstatus_state configstatus_field(admin_configstatus_parser *p, uint8_t b) {
  if (b == CONFIGSTATUS_AUTH_FIELD || b == CONFIGSTATUS_SPOOFING_FIELD) {
    p -> field = b;
    p -> state = admin_configstatus_status;
  } else {
    p -> state = admin_configstatus_error_field;
  }

  return p -> state;
}

enum admin_configstatus_state status(admin_configstatus_parser *p, uint8_t b) {
  if (b == ON || b == OFF) {
    p -> status = b;
    p -> state = admin_configstatus_done;
  } else {
    p -> state = admin_configstatus_error_status;
  }

  return p -> state;
}

enum admin_configstatus_state admin_configstatus_parser_feed(admin_configstatus_parser *p, uint8_t b) {
  switch (p -> state) {
  case admin_configstatus_action:
    p -> state = configstatus_action(p,b);
    break;
  case admin_configstatus_field:
    p -> state = configstatus_field(p,b);
    break;
  case admin_configstatus_status:
    p -> state = status(p,b);
    break;
  case admin_configstatus_error:
  case admin_configstatus_error_action:
  case admin_configstatus_error_field:
  case admin_configstatus_error_status:
  case admin_configstatus_done:
    break;
  default:
    log_print(FATAL, "Invalid state %d.\n", p->state);
    break;
  }

  return p -> state;
}

bool admin_configstatus_is_done (const enum admin_configstatus_state state, bool *err) {
  bool done;
  switch (state) {
  case admin_configstatus_error:
  case admin_configstatus_error_action:
  case admin_configstatus_error_field:
  case admin_configstatus_error_status:
    if (err != 0)
    {
      *err = true;
    }
    done = true;
    break;
  case admin_configstatus_done:
    done = true;
    break;
  default:
    done = false;
    break;
  }

  return done;
}

enum admin_configstatus_state admin_configstatus_consume (buffer *buff, struct admin_configstatus_parser *p, bool *err) {

  enum admin_configstatus_state state = p->state;

  while (buffer_can_read(buff)) {
    const uint8_t c = buffer_read(buff);
    state = admin_configstatus_parser_feed(p, c);
    if (admin_configstatus_is_done(state, err)) {
      break;
    }
  }

  return state;
}

extern int admin_configstatus_marshall (buffer *b, const uint8_t status) {
  size_t n;
  uint8_t *buff = buffer_write_ptr(b, &n);

  if (n < 1) {
    return -1;
  }
  buff[0] = status;
  buffer_write_adv(b, n);
  return 1; 
}