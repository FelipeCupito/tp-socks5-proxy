#include "../include/mng.h"


///////////////////////////////////////////
//HELLO_READ
//////////////////////////////////////////
void mng_hello_read_init(const unsigned state, struct selector_key *key) {
  struct mng *mng = ATTACHMENT(key);
  admin_connect_parser_init(&mng->parsers.connect);
}

unsigned int mng_hello_read(struct selector_key *key){
  struct mng  *mng = ATTACHMENT(key);

  unsigned int ret = HELLO_READ;
  bool err = false;

  struct admin_connect_parser *parser = mng->parsers.connect;
  buffer *buff = &mng->read_buffer;
  size_t size;
  ssize_t n;

  uint8_t *ptr = buffer_write_ptr(buff, &size);
  n = recv(key->fd, ptr, size, 0);

  if(n>0) {
    buffer_write_adv(buff, n);

    enum admin_connect_state st = admin_connect_consume(buff, parser, &err);
    if (SELECTOR_SUCCESS != selector_set_interest_key(key, OP_WRITE)) {
      err = true;
      goto finally;
    }
    if (st == admin_connect_done) {

      if (checkToken((char *) parser->password.passwd) == 0) {
        if (admin_connect_marshal(buff, STATUS_OK) == -1) { err = true; }

      } else {
        if (admin_connect_marshal(buff, STATUS_AUTH_FAIL) == -1) { err = true; }
      }
    }
  }else{
    err = true;
  }





finally:
return err ? ERROR : ret;
}


///////////////////////////////////////////
//HELLO_WRITE
//////////////////////////////////////////
unsigned int mng_hello_write(struct selector_key *key){

    return 0;
}


///////////////////////////////////////////
//REQUEST
//////////////////////////////////////////
void mng_request_init(const unsigned state, struct selector_key *key){


}
unsigned int mng_request(struct selector_key *key){

    return 0;
}

///////////////////////////////////////////
//REPLIES
//////////////////////////////////////////
unsigned int mng_replies(struct selector_key *key){

    return 0;
}
