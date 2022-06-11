#include "../include/socks5_auth.h"


///////////////////////////////////////////
//AUTH_READ
//////////////////////////////////////////
void auth_read_init(const unsigned state, struct selector_key *key){

}
unsigned int auth_read(struct selector_key *key){
    return AUTH_WRITE;
}


///////////////////////////////////////////
//AUTH_WRITE
//////////////////////////////////////////

unsigned int auth_write(struct selector_key *key){
    return REQUEST_READ;
}