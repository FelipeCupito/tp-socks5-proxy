#include "../include/mng.h"

#define MAX_RES_SIZE 255
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

  buffer_reset(&ATTACH(key)->write_buffer);
  buffer_reset(&ATTACH(key)->read_buffer);

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
  }else {err = true;}

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
        char res[MAX_RES_SIZE];
        int res_size;
        switch(parser->option) {
          case users:
            res_size = get_users(res, MAX_RES_SIZE);
            break;
          case passwords:
            res_size = get_pop3_pass(res, MAX_RES_SIZE);
            break;
          case buffsize:
            res_size = get_buff_size(res, MAX_RES_SIZE);
            break;
          case auth_status:
            res_size = get_auth_status(res, MAX_RES_SIZE);
            break;
          case spoofing_status:
            res_size = get_spoofing_status(res, MAX_RES_SIZE);
            break;
          case sent_bytes:
            res_size = _get_sent_bytes(res, MAX_RES_SIZE);
            break;
          case recv_bytes:
            res_size = _get_received_bytes(res, MAX_RES_SIZE);
            break;
          case historic_connections:
            res_size = _get_histori_conn(res, MAX_RES_SIZE);
            break;
          case current_connectios:
            res_size = _get_current_conn(res, MAX_RES_SIZE);
            break;
          default:
            res_size = 0;
            break;
        }
        if (admin_get_marshall(buff_write,parser->status, res, res_size) == -1) {err = true;}
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
  struct mng *mng = ATTACH(key);
  admin_put_parser_init(&mng->parsers.put);
}

unsigned int request_put_request(struct selector_key *key){
  struct mng  *mng = ATTACH(key);
  unsigned int ret = REQUEST_PUT;
  bool err = false;

  struct admin_put_parser *parser = &mng->parsers.put;
  buffer *buff_read = &mng->read_buffer;
  buffer *buff_write = &mng->write_buffer;
  size_t size;
  ssize_t n;

  uint8_t *ptr = buffer_write_ptr(buff_read, &size);
  n = recv(key->fd, ptr, size, 0);
  if(n>0) {
    buffer_write_adv(buff_read, n);
    enum admin_put_state st = admin_put_consume(buff_read, parser, &err);
    if(admin_put_is_done(st, &err)){
      if (SELECTOR_SUCCESS != selector_set_interest_key(key, OP_WRITE)) {
        err = true;
        goto finally;
      }
      if(st == admin_put_done){
        if(add_user((char*) parser->user.username, (char*) parser->pass.passwd) != 0){
          parser->status = 0x05;
        }
      }
      if (admin_put_marshall(buff_write, parser->status) == -1) {err = true;}
      ret = REPLIES;
    }
  }else{err = true;}

  finally:
  return err ? MNG_ERROR : ret;
}

///////////////////////////////////////////
//REQUEST_EDIT
//////////////////////////////////////////
void request_edit_init(const unsigned state, struct selector_key *key){
  struct mng *mng = ATTACH(key);
  admin_edit_parser_init(&mng->parsers.edit);
}

unsigned int request_edit_request(struct selector_key *key){
  struct mng  *mng = ATTACH(key);
  unsigned int ret = REQUEST_EDIT;
  bool err = false;

  struct admin_edit_parser *parser = &mng->parsers.edit;
  buffer *buff_read = &mng->read_buffer;
  buffer *buff_write = &mng->write_buffer;
  size_t size;
  ssize_t n;

  uint8_t *ptr = buffer_write_ptr(buff_read, &size);
  n = recv(key->fd, ptr, size, 0);
  if(n>0) {
    buffer_write_adv(buff_read, n);
    enum admin_edit_state st = admin_edit_consume(buff_read, parser, &err);
    if (admin_edit_is_done(st, &err)) {
      if (SELECTOR_SUCCESS != selector_set_interest_key(key, OP_WRITE)) {
        err = true;
        goto finally;
      }
      if (st == admin_edit_done) {
        if(edit_user((char*)parser->username, (char*)parser->value, parser->attr) != 0){
          parser->status = 0x07; //STATUS_UNKOWN_USER_FAIL
        }
      }
      if(admin_edit_marshall(buff_write, parser->status) == -1){err = true;}
      ret = REPLIES;

    }

  }else { err = true; }


  finally:
  return err ? MNG_ERROR : ret;
}

///////////////////////////////////////////
//REQUEST_BUFFSIZE
//////////////////////////////////////////
void request_buffsize_init(const unsigned state, struct selector_key *key){
  struct mng *mng = ATTACH(key);
  admin_configbuff_parser_init(&mng->parsers.buff_size);
}

unsigned int request_buffsize_request(struct selector_key *key){
  struct mng  *mng = ATTACH(key);
  unsigned int ret = REQUEST_BUFFSIZE;
  bool err = false;

  struct admin_configbuff_parser *parser = &mng->parsers.buff_size;
  buffer *buff_read = &mng->read_buffer;
  buffer *buff_write = &mng->write_buffer;
  size_t size;
  ssize_t n;

  uint8_t *ptr = buffer_write_ptr(buff_read, &size);
  n = recv(key->fd, ptr, size, 0);
  if(n>0) {
    buffer_write_adv(buff_read, n);
    enum admin_configbuff_state st = admin_configbuff_consume(buff_read, parser, &err);
    if (admin_configbuff_is_done(st, &err)) {
      if (SELECTOR_SUCCESS != selector_set_interest_key(key, OP_WRITE)) {
        err = true;
        goto finally;
      }
      if (st == admin_configbuff_done) {
        if(set_buff_size((char*) parser -> size) != 0){
          parser->status = 0x04; //STATUS_ERROR
        }
      }
      if(admin_configbuff_marshall(buff_write, parser->status) == -1){err = true;}
      ret = REPLIES;

    } else { err = true; }
  }
  finally:
  return err ? MNG_ERROR : ret;
}

///////////////////////////////////////////
//REQUEST_CONFIGSTATUS
//////////////////////////////////////////
void request_configstatus_init(const unsigned state, struct selector_key *key){
  struct mng *mng = ATTACH(key);
  admin_configstatus_parser_init(&mng->parsers.conf_status);
}

unsigned int request_configstatus_request(struct selector_key *key){
  struct mng  *mng = ATTACH(key);
  unsigned int ret = REQUEST_CONFIGSTATUS;
  bool err = false;

  struct admin_configstatus_parser *parser = &mng->parsers.conf_status;
  buffer *buff_read = &mng->read_buffer;
  buffer *buff_write = &mng->write_buffer;
  size_t size;
  ssize_t n;

  uint8_t *ptr = buffer_write_ptr(buff_read, &size);
  n = recv(key->fd, ptr, size, 0);
  if(n>0) {
    buffer_write_adv(buff_read, n);




  }else{err = true;}

  finally:
  return err ? MNG_ERROR : ret;
}

///////////////////////////////////////////
//REQUEST_DELETE
//////////////////////////////////////////
void request_delete_init(const unsigned state, struct selector_key *key){
  struct mng *mng = ATTACH(key);
  admin_delete_parser_init(&mng->parsers.delete);
}

unsigned int request_delete_request(struct selector_key *key){
  struct mng  *mng = ATTACH(key);
  unsigned int ret = REQUEST_DELETE;
  bool err = false;

  struct admin_delete_parser *parser = &mng->parsers.delete;
  buffer *buff_read = &mng->read_buffer;
  buffer *buff_write = &mng->write_buffer;
  size_t size;
  ssize_t n;

  uint8_t *ptr = buffer_write_ptr(buff_read, &size);
  n = recv(key->fd, ptr, size, 0);
  if(n>0) {
    buffer_write_adv(buff_read, n);
    enum admin_delete_state st = admin_delete_consume(buff_read, parser, &err);
    if (admin_delete_is_done(st, &err)) {
      if (SELECTOR_SUCCESS != selector_set_interest_key(key, OP_WRITE)) {
        err = true;
        goto finally;
      }
      if (st == admin_edit_done) {
        if (delete_user((char *) parser->username) != 0) {
          parser->status = 0x05; //STATUS_UNKNOWN_USER_FAIL;
        }
      }
      if (admin_delete_marshall(buff_write, parser->status) == -1) { err = true; }
      ret = REPLIES;
    }

  }else { err = true; }

  finally:
  return err ? MNG_ERROR : ret;
}


///////////////////////////////////////////
//REPLIES
//////////////////////////////////////////
unsigned int mng_replies(struct selector_key *key){
  struct mng  *mng = ATTACH(key);

  unsigned int ret = REPLIES;
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
      selector_set_interest(key->s,key->fd, OP_NOOP);
      ret = MNG_DONE;
    }
  }

  return err ? MNG_ERROR : ret;
}
