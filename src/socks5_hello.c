#include "../include/socks5_hello.h"

///////////////////////////////////////////
// HELLO_READ
//////////////////////////////////////////

static void on_hello_method(void *data, const uint8_t method) {
  uint8_t *selected = data;
  int aux = is_auth_enabled();
  if(aux){
    if(method == METHOD_AUTH_REQUIRED)
      *selected = method;
  }else{
    if(method == METHOD_NO_AUTH_REQUIRED)
      *selected = method;
  }
}

void hello_read_init(const unsigned state, struct selector_key *key) {
  hello_data *data = &ATTACHMENT(key)->client_data.hello;

  data->rb = &ATTACHMENT(key)->read_buffer;
  data->wb = &ATTACHMENT(key)->write_buffer;
  hello_parser_init(&data->parser);
  data->method = METHOD_NO_ACCEPTABLE_METHODS;
  data->parser.data = &data->method;
  data->parser.on_auth_method = on_hello_method;
}

unsigned int hello_read(struct selector_key *key) {

  hello_data *data = &ATTACHMENT(key)->client_data.hello;
  unsigned int ret = HELLO_READ;
  bool err = false;

  buffer *buff = data->rb;
  size_t size;
  ssize_t n;

  uint8_t *ptr = buffer_write_ptr(buff, &size);
  n = recv(key->fd, ptr, size, 0);

  if (n > 0) {
    buffer_write_adv(buff, n);
    
    const enum hello_state st = hello_consume(buff, &data->parser, &err);
    if (hello_is_done(st, &err)) {
      // termine de recibir ahora quiere envia la respuesta
      if (SELECTOR_SUCCESS == selector_set_interest_key(key, OP_WRITE)) {
        // serializa en un buff la respuesta al hello
        if (hello_marshall(data->wb, data->method) == -1) {err = true;}
      } else {err = true;}

      ret = HELLO_WRITE;

    } else {err = true;}
  } else {err = true;}

  return err ? ERROR : ret;
}

///////////////////////////////////////////
// HELLO_WRITE
//////////////////////////////////////////

unsigned int hello_write(struct selector_key *key) {

  hello_data *data = &ATTACHMENT(key)->client_data.hello;

  unsigned int ret = HELLO_WRITE;
  bool err = false;

  buffer *buff = data->wb;
  size_t size;
  ssize_t n;

  uint8_t *ptr = buffer_read_ptr(buff, &size);
  n = send(key->fd, ptr, size, MSG_NOSIGNAL);
  
  if (n < 0) {
    err = true;
  } else {

    buffer_read_adv(buff, n);
    if (!buffer_can_read(buff)) {
      //se mando el mensaje completo
      if (SELECTOR_SUCCESS == selector_set_interest(key->s,key->fd, OP_READ)) {
        if (data->method == METHOD_AUTH_REQUIRED && is_auth_enabled()) {
          ret = AUTH_READ;
        } else if(data->method == METHOD_NO_AUTH_REQUIRED && !is_auth_enabled()){
          ret = REQUEST_READ;
        }else{
          ret = DONE;
        }
      } else {
        // error en selector_set
        err = true;
      }
    }
  }
  return err ? ERROR : ret;
}