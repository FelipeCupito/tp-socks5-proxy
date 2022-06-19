#include "../../include/parsers/admin_put.h"

static const uint8_t STATUS_OK = 0X00;
static const uint8_t STATUS_ERROR_INVALID_ACTION = 0x01;
static const uint8_t STATUS_ERROR_INVALID_FIELD = 0x02;
static const uint8_t STATUS_ERROR_INVALID_NAMELEN = 0x03;
static const uint8_t STATUS_ERROR_INVALID_PASSLEN = 0x04;
static const uint8_t STATUS_ERROR = 0x05;

static const uint8_t PUT_ACTION = 0X01;
static const uint8_t USERS_FIELD = 0X00;

void admin_put_parser_init (struct admin_put_parser *p) {
  p -> state = admin_put_action;
  memset(&p -> user, 0, sizeof(p -> user));

  if (&p->user == NULL) {
    p -> state = admin_put_error;
    return;
  }

  memset(&p->pass, 0, sizeof(p->pass));
  if(&p->pass == NULL){
      p->state = admin_put_error;
      return;
  }

  p -> remaining = 0;
  p -> read = 0;
}

static void remaining_set(admin_put_parser *p, const int n) {
  p -> remaining = n;
  p -> read = 0;
}

static bool remaining_is_done(admin_put_parser *p) {
  return p -> read >= p -> remaining;
}

enum admin_put_state admin_put_parser_feed (admin_put_parser *p, uint8_t b){
  switch (p -> state) {
  case admin_put_action:
    if (b == PUT_ACTION) {
      p -> state = admin_put_field;
    } else {
      p -> state = admin_put_error_action;
    }
    break;
  case admin_put_field:
    if (b == USERS_FIELD) {
      p -> state = admin_put_namelen;
    } else {
      p -> state = admin_put_error_field;
    }
    break;
  case admin_put_namelen:
    if (b <= 0) {
      p -> state = admin_put_error_namelen;
    } else {
      remaining_set(p,b);
      p -> user.userlen = b;
      p -> state = admin_put_name;
    }
    break;
  case admin_put_name:
    *( (p->user.username) + p->read ) = b;
    p -> read ++;

    if (remaining_is_done(p)) {
      *( (p->user.username) + p->read ) = '\0';
      p -> state = admin_put_passlen;
    } else {
      p -> state = admin_put_name;
    }

    break;
  case admin_put_passlen:
    if (b <= 0) {
      p -> state = admin_put_error_passlen;
    } else {
      remaining_set(p,b);
      p -> pass.passlen = b;
      p -> state = admin_put_pass;
    }
    break;
  case admin_put_pass:
    *( (p->pass.passwd) + p->read ) = b;
    p -> read ++;

    if (remaining_is_done(p)) {
      *( (p->pass.passwd) + p->read ) = '\0';
      p -> state = admin_put_done;
    } else {
      p -> state = admin_put_pass;
    }
    break;
  case admin_put_done:
    p -> status = STATUS_OK;
    break;
  case admin_put_error:
    p -> status = STATUS_ERROR;
      break;
  case admin_put_error_action:
    p -> status = STATUS_ERROR_INVALID_ACTION;
      break;
  case admin_put_error_field:
    p -> status = STATUS_ERROR_INVALID_FIELD;
      break;
  case admin_put_error_namelen:
    p -> status = STATUS_ERROR_INVALID_NAMELEN;
      break;
  case admin_put_error_passlen:
    p -> status = STATUS_ERROR_INVALID_PASSLEN;
      break;
  default:
    log_print(FATAL, "Invalid state %d.\n", p->state);
    break;
  }

  return p -> state;
}

bool admin_put_is_done (const enum admin_put_state state, bool *err) {
  bool done;
  switch (state) {
  case admin_put_error:
  case admin_put_error_action:
  case admin_put_error_field:
  case admin_put_error_namelen:
  case admin_put_error_passlen:
    if (err != 0)
    {
      *err = true;
    }
    done = true;
    break;
  case admin_put_done:
    done = true;
    break;
  default:
    done = false;
    break;
  }

  return done;
}

enum admin_put_state admin_put_consume (buffer *buff, struct admin_put_parser *p, bool *err) {

  enum admin_put_state state = p->state;

  while (buffer_can_read(buff)) {
    const uint8_t c = buffer_read(buff);
    state = admin_put_parser_feed(p, c);
    if (admin_put_is_done(state, err)) {
      break;
    }
  }

  return state;
}

extern int admin_put_marshalll (buffer *b, const uint8_t status) {
  size_t n;
  uint8_t *buff = buffer_write_ptr(b, &n);

  if (n < 1) {
    return -1;
  }
  buff[0] = status;
  buffer_write_adv(b, n);
  return 1;
}