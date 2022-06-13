#include "../../include/parsers/admin_get.h"
#include "../../include/logger.h"

void admin_get_parser_init (struct admin_get_parser *p) {
  p -> state = admin_get_field;
}

enum admin_get_state field(const uint8_t b, struct admin_get_parser* p) {
  enum admin_get_state next;
  switch (b) {
    case users:
    case passwords:
    case connections:
    case bytes:
    case buffsize:
    case auth_status:
    case spoofing_status:
      p -> field = b;
      next = admin_get_option;
    break;
    default:
      next = admin_get_error_field;
    break;
  }

  return next;
}

enum admin_get_state option(const uint8_t b, struct admin_get_parser* p) {
  enum admin_get_state next;
  uint8_t f = p -> field;

  switch (b) {
    case none:
      if (f == users || f == passwords || f == buffsize || f == auth_status || f == spoofing_status) {
        p -> option = b;
        next = admin_get_done;
      } else {
        next = admin_get_error_option;
      }
      break;
    case sent_bytes:
    case received_bytes:
      if (f == bytes) {
        p -> option = b;
        next = admin_get_done;
      } else {
        next == admin_get_error_option;
      }
      break;
    case historic_connections:
    case current_connections:
      if (f == connections) {
        p -> option = b;
        next = admin_get_done;
      } else {
        next == admin_get_error_option;
      }
      break;
    default:
      next = admin_get_error;
      break;
  }

  return next;
}

enum admin_get_state admin_get_parser_feed (admin_get_parser *p, uint8_t b) {
  switch (p -> state) {
    case admin_get_field:
      p -> state = field(b,p);
      break;
    case admin_get_option:
      p -> state = option(b,p);
    case admin_get_done:
    case admin_get_error:
    case admin_get_error_field:
    case admin_get_error_option:
      break;
    default:
      log_print(FATAL, "Invalid state %d.\n", p->state);
      break;
  }

  return p -> state;
}