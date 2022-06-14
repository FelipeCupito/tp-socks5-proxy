#ifndef ADMIN_PUT_PARSER_H
#define ADMIN_PUT_PARSER_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdint.h>
#include "../buffer.h"
#include "../logger.h"

static const uint8_t STATUS_OK = 0X00;
static const uint8_t STATUS_ERROR_INVALID_ACTION = 0x01;
static const uint8_t STATUS_ERROR_INVALID_FIELD = 0x02;
static const uint8_t STATUS_ERROR_INVALID_NAMELEN = 0x03;
static const uint8_t STATUS_ERROR_INVALID_PASSLEN = 0x04;

static const uint8_t PUT_ACTION = 0X01;