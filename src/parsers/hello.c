#include "../../include/parsers/hello.h";

void hello_parser_init (hello_parser p) {
  memset(p, 0, sizeof(struct hello_parser));
}

