#include "../../include/parsers/admin_delete.h"

void admin_delete_parser_init (struct admin_delete_parser *p) {
  p -> state = admin_delete_action;
  p -> remaining = 0;
  p -> read = 0;
}

static void remaining_set(admin_delete_parser *p, const int n) {
  p -> remaining = n;
  p -> read = 0;
}

static bool remaining_is_done(admin_delete_parser *p) {
  return p -> read >= p -> remaining;
}

enum admin_delete_state delete_action(admin_delete_parser *p, uint8_t b) {
  if (b == DELETE_ACTION) {
    p -> action = b;
    p -> state = admin_delete_field;
  } else {
    p -> state = admin_delete_error_action;
  }

  return p -> state;
}

enum admin_delete_state delete_field(admin_delete_parser *p, uint8_t b) {
  if (b == USERS_FIELD) {
    p -> field = b;
    p -> state = admin_delete_keylen;
  } else {
    p -> state = admin_delete_error_field;
  }

  return p -> state;
}

enum admin_delete_state delete_key(admin_delete_parser *p, uint8_t b) {
  *( (p->key) + p->read ) = b;
    p -> read ++;

    if (remaining_is_done(p)) {
      *( (p->key) + p->read ) = '\0';
      p -> state = admin_delete_done;
    } else {
      p -> state = admin_delete_key;
    }

  return p -> state;
}

enum admin_delete_state admin_delete_parser_feed(admin_delete_parser *p, uint8_t b) {
  switch (p -> state) {
  case admin_delete_action:
    p -> state = delete_action(p,b);
    break;
  case admin_delete_field:
    p -> state = delete_field(p,b);
    break;
  case admin_delete_keylen:
    if (b <= 0) {
      p -> state = admin_delete_error_keylen;
    } else {
      remaining_set(p,b);
      p -> keylen = b;
      p -> state = admin_delete_key;
    }
    break;
  case admin_delete_key:
    p -> state = delete_key(p,b);
    break;
  case admin_delete_done:
  case admin_delete_error:
  case admin_delete_error_action:
  case admin_delete_error_field:
  case admin_delete_error_keylen:
  default:
    log_print(FATAL, "Invalid state %d.\n", p->state);
    break;
  }

  return p -> state;
}

bool admin_delete_is_done (const enum admin_delete_state state, bool *err) {
  bool done;
  switch (state) {
  case admin_delete_error:
  case admin_delete_error_action:
  case admin_delete_error_field:
  case admin_delete_error_keylen:
    if (err != 0)
    {
      *err = true;
    }
    done = true;
    break;
  case admin_delete_done:
    done = true;
    break;
  default:
    done = false;
    break;
  }

  return done;
}

enum admin_delete_state admin_delete_consume (buffer *buff, struct admin_delete_parser *p, bool *err) {

  enum admin_delete_state state = p->state;

  while (buffer_can_read(buff)) {
    const uint8_t c = buffer_read(buff);
    state = admin_delete_parser_feed(p, c);
    if (admin_delete_is_done(state, err)) {
      break;
    }
  }

  return state;
}

extern int admin_delete_marshall (buffer *b, const uint8_t status) {
  size_t n;
  uint8_t *buff = buffer_write_ptr(b, &n);

  if (n < 1) {
    return -1;
  }
  buff[0] = status;
  buffer_write_adv(b, n);
  return 1; 
}