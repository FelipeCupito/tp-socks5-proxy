#include "../../include/parsers/admin_configbuffsize.h"

void admin_configbuff_parser_init (struct admin_configbuff_parser *p) {
  p -> state = admin_configbuff_action;
  p -> remaining = 0;
  p -> read = 0;
}

static void remaining_set(admin_configbuff_parser *p, const int n) {
  p -> remaining = n;
  p -> read = 0;
}

static bool remaining_is_done(admin_configbuff_parser *p) {
  return p -> read >= p -> remaining;
}

enum admin_configbuff_state action(admin_configbuff_parser *p, uint8_t b) {
  enum admin_configbuff_state next = admin_configbuff_error_action;
  
  if (b = CONFIGBUFF_ACTION) {
    p -> action = b;
    next = admin_configbuff_sizelen;
  }

  return next;
}

enum admin_configbuff_state size(admin_configbuff_parser *p, uint8_t b) {
  enum admin_configbuff_state next = admin_configbuff_size;
  *( (p->size) + p->read ) = b;
    p -> read ++;

    if (remaining_is_done(p)) {
      *( (p->size) + p->read ) = '\0';
      next = admin_configbuff_done;
    }

  return next;
}

enum admin_configbuff_state admin_configbuff_parser_feed(admin_configbuff_parser *p, uint8_t b) {
  enum admin_configbuff_state next;
  switch (p -> state) {
  case admin_configbuff_action:
    next = action(p,b);
    break;
  case admin_configbuff_sizelen:
    if (b <= 0) {
      next = admin_configbuff_error_sizelen;
    } else {
      remaining_set(p,b);
      p -> sizelen = b;
      next = admin_configbuff_size;
    }
    break;
  case admin_configbuff_size:
    next = size(p,b);
    break;
  case admin_configbuff_error:
  case admin_configbuff_error_action:
  case admin_configbuff_error_size:
  case admin_configbuff_error_sizelen:
  case admin_configbuff_done:
    break;
  default:
    log_print(FATAL, "Invalid state %d.\n", p->state);
    break;
  }
}

bool admin_configbuff_is_done (const enum admin_configbuff_state state, bool *err) {
  bool done;
  switch (state) {
  case admin_configbuff_error:
  case admin_configbuff_error_action:
  case admin_configbuff_error_size:
  case admin_configbuff_error_sizelen:
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

extern int admin_configbuff_marshall (buffer *b, const uint8_t status) {
  size_t n;
  uint8_t *buff = buffer_write_ptr(b, &n);

  if (n < 1) {
    return -1;
  }
  buff[0] = status;
  buffer_write_adv(b, n);
  return 1; 
}