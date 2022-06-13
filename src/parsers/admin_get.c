#include "../../include/parsers/admin_get.h"
#include "../../include/logger.h"

void admin_get_parser_init (struct admin_get_parser *p) {
  p -> state = admin_get_field;
}

enum admin_get_state field(const uint8_t b, struct admin_get_parser* p) {
  switch (b) {
    case users:
    case passwords:
    case connections:
    case bytes:
    case buffsize:
    case auth_status:
    case spoofing_status:
      p -> field = b;
      return admin_get_option;
    
    default:
      return admin_get_error_field;
  }
}

enum admin_get_state admin_get_parser_feed (admin_get_parser *p, uint8_t b) {
  switch (p -> state) {
    case admin_get_field:
      p -> state = field(b,p);
      break;
  }
}