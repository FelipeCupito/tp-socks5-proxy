#include "../include/socks5_auth.h"


///////////////////////////////////////////
//AUTH_READ
//////////////////////////////////////////
void auth_read_init(const unsigned state, struct selector_key *key){
    auth_data *auth = &ATTACHMENT(key)->client_data.auth;

    auth->rb = &ATTACHMENT(key)->read_buffer;
    auth->wb = &ATTACHMENT(key)->write_buffer;

    auth_parser_init(&auth->parser);
    auth->pass = &auth->parser.pass;
    auth->user = &auth->parser.user;
}


static uint8_t check_credentials(const struct auth_data *auth){
  if(checkUser((char*)auth->user->username,(char*)auth->pass->passwd)) {
    return AUTH_SUCCESS;
  }
  return AUTH_FAIL;
}

static unsigned auth_process(struct auth_data *auth){
    unsigned ret = AUTH_WRITE;
    uint8_t status = check_credentials(auth);
    if(auth_marshall(auth->wb,status) == -1){
        ret = ERROR;
    }
    auth->status = status;
    return ret;
}

unsigned int auth_read(struct selector_key *key){
    
    auth_data *auth = &ATTACHMENT(key)->client_data.auth;
    unsigned int ret = AUTH_READ;
    bool err = false;

  buffer *buff = auth->rb;
  size_t size;
  ssize_t n;

  uint8_t *ptr = buffer_write_ptr(buff, &size);
  n = recv(key->fd, ptr, size, 0);
  if (n < 0) {
    err = true;
  } else {
    buffer_write_adv(buff, n);
    enum auth_state st = auth_consume(buff, &auth->parser, &err);
    if(auth_is_done(st, &err)){
      if(SELECTOR_SUCCESS == selector_set_interest_key(key, OP_WRITE)){
        ret = auth_process(auth);
        memcpy(ATTACHMENT(key)->user, auth->user->username, auth->user->userlen);
      }else{
        err = true;
      }
    }
  }
  return err? ERROR: ret;
}


///////////////////////////////////////////
//AUTH_WRITE
//////////////////////////////////////////
unsigned int auth_write(struct selector_key *key){
  auth_data *auth = &ATTACHMENT(key)->client_data.auth;
  unsigned int ret = AUTH_WRITE;
  buffer *buff = auth->wb;
  size_t size;
  ssize_t n;

  uint8_t *ptr = buffer_read_ptr(buff, &size);
  n = send(key->fd, ptr, size, MSG_NOSIGNAL);
  if (n < 0) {
    ret = ERROR;
  } else {
    buffer_read_adv(buff,n);
    if(!buffer_can_read(buff)){
      if(auth->status != AUTH_SUCCESS){
        ret = DONE;
      }else if(selector_set_interest_key(key,OP_READ) == SELECTOR_SUCCESS){
          ret = REQUEST_READ;
        }
       else
          ret = ERROR;
      }
    }
  return ret;
}
