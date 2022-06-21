#include "../../include/parsers/auth.h"

void auth_parser_init (struct auth_parser *p) {
  p -> state = auth_version;
  memset(&p -> user, 0, sizeof(p -> user));

  /*if (&p->user == NULL) {
    p -> state = auth_error;
    return;
  }*/

  memset(&p->pass, 0, sizeof(p->pass));
  /*if(&p->pass == NULL){
      p->state = auth_error;
      return;
  }*/

  p -> version = 0x01;
  p -> remaining = 0;
  p -> read = 0;
}

static void remaining_set(auth_parser *p, const int n) {
  p -> remaining = n;
  p -> read = 0;
}

// TODO o static int?
static bool remaining_is_done(auth_parser *p) {
  return p -> read >= p -> remaining;
}

enum auth_state auth_parser_feed(auth_parser *p, uint8_t b) {
  switch (p -> state) {
    case auth_version:
      if (p -> version == b) {
        p -> state = auth_userlen;
      } else {
        p -> state = auth_error_unsupported_version;
      }
      
      break;
    
    case auth_userlen:
      if (b <= 0) {
        p -> state = auth_error_user_len;
      } else {
        remaining_set(p,b);
        p -> user.userlen = b;
        p -> state = auth_username;
      }

      break;

    case auth_username:
      // escribi el byte al final de username
      *( (p->user.username) + p->read ) = b;
      p -> read ++;

      if (remaining_is_done(p)) {
        *( (p->user.username) + p->read ) = '\0';
        p -> state = auth_passlen;
      } else {
        p -> state = auth_username;
      }

      break;
    
    case auth_passlen:
      if (b <= 0) {
        p -> state = auth_error_pass_len;
      } else {
        remaining_set(p,b);
        p -> pass.passlen = b;
        p -> state = auth_pass;
      }

      break;

    case auth_pass:
      // escribi el byte al final de pass
      *( (p->pass.passwd) + p->read ) = b;
      p -> read ++;

      if (remaining_is_done(p)) {
        *( (p->pass.passwd) + p->read ) = '\0';
        p -> state = auth_done;
      } else {
        p -> state = auth_pass;
      }

      break;
    case auth_error:
    case auth_error_pass_len:
    case auth_error_unsupported_version:
    case auth_error_user_len:
    case auth_done:
      break;
    default:
      log_print(FATAL, "Invalid state %d.\n", p->state);
      break;
  }

  return p -> state;
}

bool auth_is_done(const enum auth_state state, bool *errored)
{
  bool done;
  switch (state)
  {
  case auth_error_unsupported_version:
  case auth_error:
  case auth_error_pass_len:
  case auth_error_user_len:
    if (errored != 0)
    {
      *errored = true;
    }
    done = true;
    break;
  case auth_done:
    done = true;
    break;
  default:
    done = false;
    break;
  }

  return done;
}

enum auth_state auth_consume(buffer *buff, struct auth_parser *p, bool *errored) {
  enum auth_state state = p->state;

  while (buffer_can_read(buff)) {
    const uint8_t c = buffer_read(buff);
    state = auth_parser_feed(p, c);
    if (auth_is_done(state, errored)) {
      break;
    }
  }

  return state;
}


int auth_marshall(buffer *b, const uint8_t status) {
  size_t n;
  uint8_t *buff = buffer_write_ptr(b, &n);

  if (n < 2)
  {
    return -1;
  }

  buff[0] = 0x01;
  buff[1] = status;
  buffer_write_adv(b, 2);
  return 2;
}
