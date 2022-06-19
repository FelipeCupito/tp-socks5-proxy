#include "../../include/parsers/admin_configbuffsize.h"


static const uint8_t STATUS_OK                      = 0x00;
static const uint8_t STATUS_ERROR_INVALID_ACTION    = 0x01;
static const uint8_t STATUS_ERROR_INVALID_SIZELEN   = 0x02;
static const uint8_t STATUS_ERROR_INVALID_SIZE      = 0x03;
static const uint8_t STATUS_ERROR                   = 0x04;

static const uint8_t CONFIGBUFF_ACTION = 0x03;

void admin_configbuff_parser_init (struct admin_configbuff_parser *p) {
  p -> state = admin_configbuff_action;
  p -> remaining = 4;
  p -> read = 0;
}

static bool remaining_is_done(admin_configbuff_parser *p) {
  return p -> read >= p -> remaining;
}

enum admin_configbuff_state configbuff_action(admin_configbuff_parser *p, uint8_t b) {
  if (b == CONFIGBUFF_ACTION) {
    p -> action = b;
    p -> state = admin_configbuff_size;
  } else {
    p -> state = admin_configbuff_error_action;
  }

  return p -> state;
}

enum admin_configbuff_state size(admin_configbuff_parser *p, uint8_t b) {
  int i = p -> read;
  log_print(INFO, "byte %d.\n", b);
  p -> size[i] = b;
  p -> read ++;

  if (remaining_is_done(p)) {
    uint32_t aux_size = p -> size[3] | (p -> size[2] << 8) | (p -> size[1] << 16) | (p -> size[0] << 24);
    if (aux_size <= 0) {
      p -> state = admin_configbuff_error_size;
    } else {
      p -> state = admin_configbuff_done;
    }
  }

  return p -> state;
}

enum admin_configbuff_state admin_configbuff_parser_feed(admin_configbuff_parser *p, uint8_t b) {
  switch (p -> state) {
  case admin_configbuff_action:
    p -> state = configbuff_action(p,b);
    break;
  case admin_configbuff_size:
    p -> state = size(p,b);
    break;
  case admin_configbuff_error:
    p -> status = STATUS_ERROR;
    break;
  case admin_configbuff_error_action:
  p -> status = STATUS_ERROR_INVALID_ACTION;
    break;
  case admin_configbuff_error_size:
  p -> status = STATUS_ERROR_INVALID_SIZE;
    break;
  case admin_configbuff_error_sizelen:
  p -> status = STATUS_ERROR_INVALID_SIZELEN;
    break;
  case admin_configbuff_done:
  p -> status = STATUS_OK;
    break;
  default:
    log_print(FATAL, "Invalid state %d.\n", p->state);
    break;
  }

  return p -> state;
}

bool admin_configbuff_is_done (const enum admin_configbuff_state state, bool *err) {
  bool done;
  switch (state) {
  case admin_configbuff_error:
  case admin_configbuff_error_action:
  case admin_configbuff_error_size:
    if (err != 0)
    {
      *err = true;
    }
    done = true;
    break;
  case admin_configbuff_done:
    done = true;
    break;
  default:
    done = false;
    break;
  }

  return done;
}

enum admin_configbuff_state admin_configbuff_consume (buffer *buff, struct admin_configbuff_parser *p, bool *err) {

  enum admin_configbuff_state state = p->state;

  while (buffer_can_read(buff)) {
    const uint8_t c = buffer_read(buff);
    state = admin_configbuff_parser_feed(p, c);
    if (admin_configbuff_is_done(state, err)) {
      break;
    }
  }

  return state;
}

extern int admin_configbuff_marshalll (buffer *b, const uint8_t status) {
  size_t n;
  uint8_t *buff = buffer_write_ptr(b, &n);

  if (n < 1) {
    return -1;
  }
  buff[0] = status;
  buffer_write_adv(b, n);
  return 1; 
}