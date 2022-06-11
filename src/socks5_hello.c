#include "../include/socks5_hello.h"


///////////////////////////////////////////
//HELLO_READ
//////////////////////////////////////////

void hello_read_init(const unsigned state, struct selector_key *key){
    // TODO: init
}

void hello_read_close(const unsigned state, struct selector_key *key){
    // TODO: close
}
unsigned int hello_read(struct selector_key *key){
    // TODO: When parse branch is merged
    return REQUEST_READ;
}


///////////////////////////////////////////
//HELLO_WRITE
//////////////////////////////////////////

unsigned int hello_write(struct selector_key *key){
    return AUTH_READ;
}