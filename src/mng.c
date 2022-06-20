#include "../include/mng.h"


///////////////////////////////////////////
//CONNECT_READ
//////////////////////////////////////////
void mng_connect_read_init(const unsigned state, struct selector_key *key) {
  struct mng *mng = ATTACH(key);
  admin_connect_parser_init(&mng->parsers.connect);
}

unsigned int mng_connect_read(struct selector_key *key){
  struct mng  *mng = ATTACH(key);

  unsigned int ret = CONNECT_READ;
  bool err = false;

  struct admin_connect_parser *parser = &mng->parsers.connect;
  buffer *buff_read = &mng->read_buffer;
  buffer *buff_write = &mng->write_buffer;
  size_t size;
  ssize_t n;

  uint8_t *ptr = buffer_write_ptr(buff_read, &size);
  n = recv(key->fd, ptr, size, 0);

  if(n>0) {
    buffer_write_adv(buff_read, n);

    enum admin_connect_state st = admin_connect_consume(buff_read, parser, &err);
    if (admin_connect_is_done(st, &err)) {
      if (SELECTOR_SUCCESS != selector_set_interest_key(key, OP_WRITE)) {
        err = true;
        goto finally;
      }
      if(st == admin_connect_done){
        if (checkToken((char *) parser->password.passwd)) {
          //la contraseña es correcta
          if (admin_connect_marshall(buff_write, 0x00) == -1) {err = true;}
        }else{
          //la contraseña es incorrecta
          ATTACH(key)->status = CLOSE;
          if (admin_connect_marshall(buff_write, 0X04) == -1) {err = true;}
        }
      }else{
        ATTACH(key)->status = CLOSE;
        if (admin_connect_marshall(buff_write, parser->status) == -1) { err = true; }
      }
      ret = CONNECT_WRITE;
    }
  }else{
    err = true;
  }

finally:
return err ? MNG_ERROR : ret;
}


///////////////////////////////////////////
//CONNECT_WRITE
//////////////////////////////////////////
unsigned int mng_connect_write(struct selector_key *key){
  struct mng  *mng = ATTACH(key);

  unsigned int ret = CONNECT_WRITE;
  bool err = false;

  buffer *buff = &mng->write_buffer;
  size_t size;
  ssize_t n;

  uint8_t *ptr = buffer_read_ptr(buff, &size);
  n = send(key->fd, ptr, size, MSG_NOSIGNAL);

  if (n <= 0) {
    err = true;
  } else {
    buffer_read_adv(buff, n);
    if (!buffer_can_read(buff)) {
      if(ATTACH(key)->status != CLOSE){
        //no se cierra
        if (SELECTOR_SUCCESS == selector_set_interest(key->s,key->fd, OP_READ)) {
          ret = REQUEST;
        }else{
          err = true;
        }
      }else{
        ret = MNG_DONE;
      }
    }
  }

  return err ? MNG_ERROR : ret;
}


///////////////////////////////////////////
//REQUEST
//////////////////////////////////////////
unsigned int mng_request(struct selector_key *key){

  unsigned int ret = REQUEST;
  bool err = false;
  
  buffer *buff_read = &ATTACH(key)->read_buffer;
  size_t size;
  ssize_t n;

  uint8_t *ptr = buffer_write_ptr(buff_read, &size);
  if(size < 2){
    err = true;
    goto finally;
  }

  n = recv(key->fd, ptr, 1, 0);

  if(n == 1) {
    buffer_write_adv(buff_read, n);
    switch (*ptr) {
      case GET:
        ret = REQUEST_GET;
        break;
      case PUT:
        ret = REQUEST_PUT;
        break;
      case EDIT:
        ret = REQUEST_EDIT;
        break;
      case CONFIGBUFFSIZE:
        ret = REQUEST_BUFFSIZE;
        break;
      case CONFIGSTATUS:
        ret = REQUEST_CONFIGSTATUS;
        break;
      case DELETE:
        ret = REQUEST_DELETE;
        break;
    }
  }
  finally:
    return err ? MNG_ERROR : ret;
}
////////////////////////////////////////////
//REQUEST_GET
////////////////////////////////////////////
void request_get_init(const unsigned state, struct selector_key *key){
  struct mng *mng = ATTACH(key);
  admin_get_parser_init(&mng->parsers.get);

}
unsigned int request_get_request(struct selector_key *key){
  struct mng  *mng = ATTACH(key);
  unsigned int ret = REQUEST_GET;
  bool err = false;

  struct admin_get_parser *parser = &mng->parsers.get;
  buffer *buff_read = &mng->read_buffer;
  buffer *buff_write = &mng->write_buffer;
  size_t size;
  ssize_t n;

  uint8_t *ptr = buffer_write_ptr(buff_read, &size);
  n = recv(key->fd, ptr, size, 0);

  if(n>0) {
    buffer_write_adv(buff_read, n);

    enum admin_get_state st = admin_get_consume(buff_read, parser, &err);
    if(admin_get_is_done(st, &err)){
      if (SELECTOR_SUCCESS != selector_set_interest_key(key, OP_WRITE)) {
        err = true;
        goto finally;
      }

      if(st == admin_get_done ){
        uint8_t res;
        switch(parser->option) {
          //todo: que pasa si no puede escribir toda la respuesta
          case users:
            res = get_users(buff_write);
            break;
          case passwords:
            res = get_pass(buff_write);
            break;
          case buffsize:
            res = get_buff_size(buff_write);
            break;
          case auth_status:
            res = get_auth_status(buff_write);
            break;
          case spoofing_status:
            res = get_spoofing_status(buff_write);
            break;
          case sent_bytes:
            res = _get_sent_bytes(buff_write);
            break;
          case recv_bytes:
            res = _get_received_bytes(buff_write);
            break;
          case historic_connections:
            res = _get_histori_conn(buff_write);
            break;
          case current_connectios:
            res = _get_current_conn(buff_write);
            break;
          default:
            res = 0;
            break;
        }
        if (admin_get_marshall(buff_write,parser->status, &res) == -1) {err = true;}
      }
      ret = REPLIES;
    }
  }else{err = true;}

  finally:
  return err ? MNG_ERROR : ret;
}

///////////////////////////////////////////
//REQUEST_PUT
//////////////////////////////////////////
void request_put_init(const unsigned state, struct selector_key *key){

}
unsigned int request_put_request(struct selector_key *key){

}

///////////////////////////////////////////
//REQUEST_EDIT
//////////////////////////////////////////
void request_edit_init(const unsigned state, struct selector_key *key){

}
unsigned int request_edit_request(struct selector_key *key){

}

///////////////////////////////////////////
//REQUEST_BUFFSIZE
//////////////////////////////////////////
void request_buffsize_init(const unsigned state, struct selector_key *key){

}
unsigned int request_buffsize_request(struct selector_key *key){

}

///////////////////////////////////////////
//REQUEST_CONFIGSTATUS
//////////////////////////////////////////
void request_configstatus_init(const unsigned state, struct selector_key *key){

}
unsigned int request_configstatus_request(struct selector_key *key){

}

///////////////////////////////////////////
//REQUEST_DELETE
//////////////////////////////////////////
void request_delete_init(const unsigned state, struct selector_key *key){

}
unsigned int request_delete_request(struct selector_key *key){

}


///////////////////////////////////////////
//REPLIES
//////////////////////////////////////////
unsigned int mng_replies(struct selector_key *key){

    return 0;
}
