#ifndef ADMIN_SET_PARSER_H
#define ADMIN_SET_PARSER_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdint.h>
#include "../buffer.h"
#include "../logger.h"

enum field {
  users = 0x00,
  passwords,
  buffsize,
  auth_status,
  spoofing_status,
};

enum admin_set_state {
  admin_set_field,
  admin_set_option,
  admin_set_done,
  admin_set_error,
  admin_set_error_field,
  admin_set_error_option,
};

#endif