#include "../../include/parsers/admin_connect.h"

static const uint8_t STATUS_OK = 0X00;
static const uint8_t STATUS_ERROR_IN_VERSION = 0x01;
static const uint8_t STATUS_ERROR_IN_PASSLEN = 0X02;
static const uint8_t STATUS_ERROR = 0X03;
static const uint8_t STATUS_AUTH_FAIL = 0X04;

void admin_connect_parser_init (struct admin_connect_parser *p) {
  p -> state  = admin_connect_version;
  memset(&p -> password, 0, sizeof(p -> password));

  if (&p->password == NULL) {
    p -> state = admin_connect_error;
    return;
  }

  p -> version = 0x00;
  p -> remaining = 0;
  p -> read = 0;
}

static void remaining_set(admin_connect_parser *p, const int n) {
  p -> remaining = n;
  p -> read = 0;
}

static bool remaining_is_done(admin_connect_parser *p) {
  return p -> read >= p -> remaining;
}

enum admin_connect_state admin_connect_parser_feed(admin_connect_parser *p, uint8_t b) {
  switch (p -> state) {
    case admin_connect_version:
      if (p -> version == b) {
        p -> state = admin_connect_passlen;
      } else {
        p -> state = admin_connect_error_version;
      }
      break;
    case admin_connect_passlen:
      if (b <= 0) {
        p -> state = admin_connect_error_passlen;
      } else {
        remaining_set(p,b);
        p -> password.passlen = b;
        p -> state = admin_connect_passwd;
      }
      break;
    case admin_connect_passwd:
      *( (p->password.passwd) + p->read ) = b;
      p -> read ++;

      if (remaining_is_done(p)) {
        *( (p->password.passwd) + p->read ) = '\0';
        p -> state = admin_connect_done;
      }
      break;
    case admin_connect_done:
      p -> status = STATUS_OK;
      break;
    case admin_connect_error:
      p -> status = STATUS_ERROR;
      break;
    case admin_connect_error_version:
      p -> status = STATUS_ERROR_IN_VERSION;
        break;
    case admin_connect_error_passlen:
      p -> status = STATUS_ERROR_IN_PASSLEN;
        break;
      break;

    default:
      log_print(FATAL, "Invalid state %d.\n", p->state);
      break;
  }

  return p -> state;
}

bool admin_connect_is_done (const enum admin_connect_state state, bool *errored) {
  bool done;

  switch (state) {
    case admin_connect_error:
    case admin_connect_error_passlen:
    case admin_connect_error_version:
      if (errored != 0) {
        *errored = true;
      }
    case admin_connect_done:
      done = true;
      break;
    default:
      done = false;
      break;
  }

  return done;
}

enum admin_connect_state admin_connect_consume(buffer *buff, struct admin_connect_parser *p, bool *errored) {
  enum admin_connect_state state = p->state;

  while (buffer_can_read(buff)) {
    const uint8_t c = buffer_read(buff);
    state = admin_connect_parser_feed(p, c);
    if (admin_connect_is_done(state, errored)) {
      break;
    }
  }

  return state;
}

extern int admin_connect_marshall(buffer *b, const uint8_t status) {
  size_t n;
  uint8_t *buff = buffer_write_ptr(b, &n);

  if (n < 1) {
    return -1;
  }

  buff[0] = status;
  buffer_write_adv(b, 1);
  return 1;
}