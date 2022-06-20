#include "../../include/parsers/admin_delete.h"

static const uint8_t STATUS_OK                      = 0X00;
static const uint8_t STATUS_ERROR_INVALID_ACTION    = 0x01;
static const uint8_t STATUS_ERROR_INVALID_FIELD     = 0x02;
static const uint8_t STATUS_ERROR_INVALID_ULEN      = 0x03;
static const uint8_t STATUS_ERROR                   = 0x04;
static const uint8_t STATUS_UNKNOWN_USER_FAIL       = 0x05; //TODO: no se usa

static const uint8_t DELETE_ACTION = 0x05;
static const uint8_t USERS_FIELD = 0x00;

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
    p -> status = STATUS_ERROR_INVALID_ACTION;
  }

  return p -> state;
}

enum admin_delete_state delete_field(admin_delete_parser *p, uint8_t b) {
  if (b == USERS_FIELD) {
    p -> field = b;
    p -> state = admin_delete_ulen;
  } else {
    p -> state = admin_delete_error_field;
    p -> status = STATUS_ERROR_INVALID_FIELD;
  }

  return p -> state;
}

enum admin_delete_state delete_username(admin_delete_parser *p, uint8_t b) {
  *( (p->username) + p->read ) = b;
    p -> read ++;

    if (remaining_is_done(p)) {
      *( (p->username) + p->read ) = '\0';
      p -> state = admin_delete_done;
      p -> status = STATUS_OK;
    } else {
      p -> state = admin_delete_username;
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
  case admin_delete_ulen:
    if (b <= 0) {
      p -> state = admin_delete_error_ulen;
      p -> status = STATUS_ERROR_INVALID_ULEN;
    } else {
      remaining_set(p,b);
      p -> ulen = b;
      p -> state = admin_delete_username;
    }
    break;
  case admin_delete_username:
    p -> state = delete_username(p,b);
    break;
  case admin_delete_done:
    p -> status = STATUS_OK;
    break;
  case admin_delete_error:
  p -> status = STATUS_ERROR;
    break;
  case admin_delete_error_action:
  p -> status = STATUS_ERROR_INVALID_ACTION;
    break;
  case admin_delete_error_field:
  p -> status = STATUS_ERROR_INVALID_FIELD;
    break;
  case admin_delete_error_ulen:
  p -> status = STATUS_ERROR_INVALID_ULEN;
    break;
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
  case admin_delete_error_ulen:
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

extern int admin_delete_marshalll (buffer *b, const uint8_t status) {
  size_t n;
  uint8_t *buff = buffer_write_ptr(b, &n);

  if (n < 1) {
    return -1;
  }
  buff[0] = status;
  buffer_write_adv(b, n);
  return 1; 
}