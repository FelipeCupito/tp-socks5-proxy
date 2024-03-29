#include "../../include/parsers/admin_edit.h"

static const uint8_t STATUS_OK                      = 0X00;
static const uint8_t STATUS_ERROR_INVALID_ACTION    = 0x01;
static const uint8_t STATUS_ERROR_INVALID_FIELD     = 0x02;
static const uint8_t STATUS_ERROR_INVALID_ULEN      = 0x03;
static const uint8_t STATUS_ERROR_INVALID_ATTRIBUTE = 0x04;
static const uint8_t STATUS_ERROR_INVALID_VALUELEN  = 0x05;
static const uint8_t STATUS_ERROR                   = 0x06;
const uint8_t STATUS_UNKOWN_USER_FAIL = 0x07;

static const uint8_t EDIT_ACTION = 0x02;
static const uint8_t USERS_FIELD = 0x00;

static const uint8_t USERNAME_ATTRIBUTE = 0x00;
static const uint8_t PASSWORD_ATTRIBUTE = 0x01;


void admin_edit_parser_init (struct admin_edit_parser *p) {
  p -> state = admin_edit_action;
  p -> remaining = 0;
  p -> read = 0;
}

static void remaining_set(admin_edit_parser *p, const int n) {
  p -> remaining = n;
  p -> read = 0;
}

static bool remaining_is_done(admin_edit_parser *p) {
  return p -> read >= p -> remaining;
}

enum admin_edit_state edit_action(admin_edit_parser *p, uint8_t b) {
  if (b == EDIT_ACTION) {
    p -> action = b;
    p -> state = admin_edit_field;
  } else {
    p -> state = admin_edit_error_action;
    p -> status = STATUS_ERROR_INVALID_ACTION;
  }

  return p -> state;
}

enum admin_edit_state edit_field(admin_edit_parser *p, uint8_t b) {
  if (b == USERS_FIELD) {
    p -> field = b;
    p -> state = admin_edit_ulen;
  } else {
    p -> state = admin_edit_error_field;
    p -> status = STATUS_ERROR_INVALID_FIELD;
  }

  return p -> state;
}

enum admin_edit_state edit_username(admin_edit_parser *p, uint8_t b) {
  *( (p->username) + p->read ) = b;
    p -> read ++;

    if (remaining_is_done(p)) {
      *( (p->username) + p->read ) = '\0';
      p -> state = admin_edit_attribute;
    } else {
      p -> state = admin_edit_username;
    }

  return p -> state;
}

enum admin_edit_state attribute(admin_edit_parser *p, uint8_t b) {
 if (b == USERNAME_ATTRIBUTE || b == PASSWORD_ATTRIBUTE) {
   p -> attr = b;
   p -> state = admin_edit_valuelen;
 } else {
   p -> state = admin_edit_error_attribute;
   p -> status = STATUS_ERROR_INVALID_ATTRIBUTE;
 }

 return p -> state;
}

enum admin_edit_state edit_value(admin_edit_parser *p, uint8_t b) {
  *( (p->value) + p->read ) = b;
    p -> read ++;

    if (remaining_is_done(p)) {
      *( (p->value) + p->read ) = '\0';
      p -> state = admin_edit_done;
      p -> status = STATUS_OK;
    } else {
      p -> state = admin_edit_value;
    }

  return p -> state;
}

enum admin_edit_state admin_edit_parser_feed(admin_edit_parser *p, uint8_t b) {
  switch (p -> state) {
  case admin_edit_action:
    p -> state = edit_action(p,b);
    break;
  case admin_edit_field:
    p -> state = edit_field(p,b);
    break;
  case admin_edit_ulen:
    if (b <= 0) {
      p -> state = admin_edit_error_ulen;
      p -> status = STATUS_ERROR_INVALID_ULEN;
    } else {
      remaining_set(p,b);
      p -> ulen = b;
      p -> state = admin_edit_username;
    }
    break;
  case admin_edit_username:
    p -> state = edit_username(p,b);
    break;
  case admin_edit_attribute:
    p -> state = attribute(p,b);
    break;
  case admin_edit_valuelen:
    if (b <= 0) {
      p -> state = admin_edit_error_valuelen;
      p -> status = STATUS_ERROR_INVALID_VALUELEN;
    } else {
      remaining_set(p,b);
      p -> valuelen = b;
      p -> state = admin_edit_value;
    }
    break;
  case admin_edit_value:
    p -> state = edit_value(p,b);
    break;
  case admin_edit_done:
    p -> status = STATUS_OK;
    break;
  case admin_edit_error:
  p -> status = STATUS_ERROR;
    break;
  case admin_edit_error_action:
  p -> status = STATUS_ERROR_INVALID_ACTION;
    break;
  case admin_edit_error_field:
  p -> status = STATUS_ERROR_INVALID_FIELD;
    break;
  case admin_edit_error_ulen:
  p -> status = STATUS_ERROR_INVALID_ULEN;
    break;
  case admin_edit_error_valuelen:
  p -> status = STATUS_ERROR_INVALID_VALUELEN;
    break;
  case admin_edit_error_attribute:
  p -> status = STATUS_ERROR_INVALID_ATTRIBUTE;
    break;
  default:
    log_print(FATAL, "Invalid state %d.\n", p->state);
    break;
  }

  return p -> state;
}

bool admin_edit_is_done (const enum admin_edit_state state, bool *err) {
  bool done;
  switch (state) {
  case admin_edit_error:
  case admin_edit_error_action:
  case admin_edit_error_field:
  case admin_edit_error_ulen:
  case admin_edit_error_valuelen:
  case admin_edit_error_attribute:
    if (err != 0)
    {
      *err = true;
    }
    done = true;
    break;
  case admin_edit_done:
    done = true;
    break;
  default:
    done = false;
    break;
  }

  return done;
}

enum admin_edit_state admin_edit_consume (buffer *buff, struct admin_edit_parser *p, bool *err) {

  enum admin_edit_state state = p->state;

  while (buffer_can_read(buff)) {
    const uint8_t c = buffer_read(buff);
    state = admin_edit_parser_feed(p, c);
    if (admin_edit_is_done(state, err)) {
      break;
    }
  }

  return state;
}

int admin_edit_marshall(buffer *b, const uint8_t status) {
  size_t n;
  uint8_t *buff = buffer_write_ptr(b, &n);

  if (n < 1) {
    return -1;
  }
  buff[0] = status;
  buffer_write_adv(b, n);
  return 1; 
}