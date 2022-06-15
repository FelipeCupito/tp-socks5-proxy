#include "../../include/management_protocol/management_protocol_handler.h"

char* token = "somos_grupo_6";

void login(int fd, char* password) {
    // Iniciamos parser
    // struct admin_connect_parser *parser = malloc(sizeof(struct admin_connect_parser));
    // admin_connect_parser_init(&parser);
    // parser->state = admin_connect_version;
    // parser->version = 
    // admin_connect_parser_feed(parser, )

    if (strcmp(password, token) == 0) {
        // TODO:
    }
    else {
        // ERROR
    }
}