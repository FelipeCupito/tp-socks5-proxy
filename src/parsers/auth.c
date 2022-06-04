#include "../../include/parsers/auth.h"

/* ESTADOS

  auth_version,
  auth_userlen,
  auth_username,
  auth_passlen,
  auth_pass,
  auth_success,
  auth_error_unsupported_version,
  auth_error,
  // auth_error_user_len,
  // auth_error_pass_len,

*/

void auth_parser_init (struct auth_parser *p, enum auth_type type) {
  p -> state = auth_version;
  memset(&p -> user, 0, sizeof(p -> user));

  if (&p->user == NULL) {
    p -> state = auth_error;
    return;
  }

  memset(&p->pass, 0, sizeof(p->pass));
  if(&p->pass == NULL){
      p->state = auth_error;
      return;
  }

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

        // TODO es necesario esto?
        // if(p->usr.uname == NULL)
        // {
        //     p->state = auth_error;
        //     return p->state;
        // }

        p -> state = auth_username;
      }

      break;

    case auth_username:
      // escribi el byte al final de username
      *( (p->user.username) + p->read ) = b;
      p -> read ++;

      if (remaining_is_done(p)) {
        *( (p->usr.uname) + p->read ) = '\0';
        p -> state = auth_passlen;
      } else {
        p -> state = auth_userlen;
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
    
  }
}
