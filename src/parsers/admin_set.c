#include "../../include/parsers/admin_set.h"

void admin_set_parser_init (struct admin_set_parser *p) {
  p -> state = admin_set_field;
}

enum admin_set_state field(const uint8_t b, struct admin_set_parser* p) {
  enum admin_set_state next;
  switch (b) {
    case buffsize:
    case auth_status:
    case spoofing_status:
      p -> field = b;
      next = admin_set_valuelen;
    break;
    default:
      next = admin_set_error_field;
    break;
  }

  return next;
}

enum admin_set_state admin_set_parser_feed(admin_set_parser *p, uint8_t b) {
  enum admin_set_state next;

  switch (p -> state) {
  case admin_set_field:
    next = field(b,p);
    break;
  case admin_set_valuelen:
    if (b > 0) {
      p -> valuelen = b;
      next = admin_set_value;
    } else {
      next = admin_set_error_valuelen;
    }
    break;
  case admin_set_value:
    break;
  default:
    break;
  }
}