#include "../../include/management_protocol/management_protocol_handler.h"

char* token = "somos_grupo_6";

void login(int fd, char* password) {
    // Iniciamos parser
    // struct admin_connect_parser *parser = malloc(sizeof(struct admin_connect_parser));
    // admin_connect_parser_init(&parser);
    // parser->state = admin_connect_version;
    // parser->version = 
    // admin_connect_parser_feed(parser, )

    // Creamos primer mensaje
    size_t password_len = strlen(password);
    char* msg = malloc(password_len + 3);
    msg[0] = 0x00;
    msg[1] = password_len;
    strcpy((char *)(msg + 2), password);
    
    // using send due to connected state
    send(fd, msg, strlen(msg), MSG_NOSIGNAL);   // MSG_NOSIGNAL -> don't generate a SIGPIPE
    
    if (strcmp(password, token) == 0) {
        // TODO:
    }
    else {
        // ERROR
    }
}

void getHistoricalConnections() {

}

void getConcurrentConections() {

}

void getSentBytes() {

}

void getReceivedBytes() {
    
}