#include "../include/socks5_request.h"

// privadas:
unsigned int request_connect(struct selector_key *key);
unsigned request_process(struct selector_key *key, request_data *data);


///////////////////////////////////////////
// REQUEST_READ
//////////////////////////////////////////
void request_init(const unsigned state, struct selector_key *key) {

  request_data *data = &ATTACHMENT(key)->client_data.request;

  //parser
  data->rb = &ATTACHMENT(key)->read_buffer;
  data->wb = &ATTACHMENT(key)->write_buffer;
  data->parser.request = &data->request;
  data->status = status_general_SOCKS_server_failure;
  request_parser_init(&data->parser);
  
  //fd
  data->client_fd = &ATTACHMENT(key)->client_fd;
  data->final_server_fd = &ATTACHMENT(key)->final_server_fd;

  //addr
  data->final_server_addr = &ATTACHMENT(key)->final_server_addr;
  data->final_server_len = &ATTACHMENT(key)->final_server_len;
  //data->server_domain = &ATTACHMENT(key)->server_domain;
}

unsigned int request_read(struct selector_key *key) {

  request_data *data = &ATTACHMENT(key)->client_data.request;
  unsigned int ret = REQUEST_READ;
  bool err = 0;

  buffer *buff = data->rb;
  size_t size;
  ssize_t n;

  uint8_t *ptr = buffer_write_ptr(buff, &size);
  n = recv(key->fd, ptr, size, 0);
  if(n < 0){
    err = true;
  }else{
    buffer_write_adv(buff, n);
    request_state st = request_consume(buff, &data->parser, &err);
    if(request_is_done(st, 0)){
      ret = request_process(key, data);
    }
  }
  return err ? ERROR : ret;
}

//Privadas:
unsigned request_process(struct selector_key *key, request_data *data){
  
  socks5 *socks5= ATTACHMENT(key);
  unsigned int ret = REQUEST_CONNECTING;

  struct sockaddr_in addr4;
  struct sockaddr_in6 addr6;
  ssize_t len;


  if(data->request.cmd == socks_req_cmd_connect ){
    switch (data->request.dst_addr_type){    
    
    case socks_req_addrtype_ipv4:
      addr4 = data->request.dst_addr.ipv4;
      addr4.sin_family = AF_INET;
      addr4.sin_port = data->request.dst_port;

      len = sizeof(data->request.dst_addr.ipv4);
      socks5->final_server_len = len;
      memcpy(&socks5->final_server_addr, &addr4, len);
      
      ret = request_connect(key);
      break;

    case socks_req_addrtype_ipv6:
      addr6 = data->request.dst_addr.ipv6;
      addr6.sin6_family = AF_INET6;
      addr6.sin6_port = data->request.dst_port; 

      len = sizeof(data->request.dst_addr.ipv6);
      socks5->final_server_len = len;
      memcpy(&socks5->final_server_addr, &addr6, len);
      
      ret = request_connect(key);
      break;

    case socks_req_addrtype_domain:
      /*
        TODO:
      */
      break;
    
    default:
      data->status = request_error_unsupported_addresstype;
      ret = REQUEST_WRITE;
      break;
    }
  }else{
    data->status = request_error_unsupported_cmd;
    ret = REQUEST_WRITE;
  }
  return ret;
}

unsigned int request_connect(struct selector_key *key) {
  unsigned int ret = REQUEST_CONNECTING;
  bool err = false;

  struct socks5 *socks5 = ATTACHMENT(key);
  int *fd = &socks5->final_server_fd;


  if (*fd != -1) {
    //no es la primera vez que intentamos conectarnos
    selector_unregister_fd(key->s, *fd);
    close(*fd);
    //TODO:
    // intentamos con otra IP
  }

  //creo el socket  
  *fd = socket(socks5->final_server_addr.ss_family, SOCK_STREAM, IPPROTO_TCP);
  if (*fd == -1) {
    err = true;
    goto finally;
  }

//hago que fd sea no bloqueante
  if (selector_fd_set_nio(*fd) == -1) {
    err = true;
    goto finally;
  }

  struct sockaddr_storage *sin = (struct sockaddr_storage *)&socks5->final_server_addr;
  socklen_t addrlen = socks5->final_server_len;

  if (connect(*fd, (struct sockaddr *)sin, addrlen) == -1) {

    selector_status ss = SELECTOR_SUCCESS;
    request_data *request = &socks5->client_data.request;
    enum socks_response_status st;

    switch (errno) {
      case EINPROGRESS:
        ss = selector_register(key->s, *fd, &socks5_handler, OP_WRITE, ATTACHMENT(key));
        if (ss != SELECTOR_SUCCESS) {
          err = true;
          goto finally;
        }
        ss = selector_set_interest(key->s, socks5->client_fd , OP_NOOP);
        if (ss != SELECTOR_SUCCESS) {
          err = true;
          goto finally;
        }
        break;
      default:
        //hubo un error:
        //TODO: paso a otra ip o error

        //si no hay mas ip, comunico el error y TODO:cierro conecion
        st = errno_to_socks(errno);
        socks5->status = st;
        if(request_marshall(request->wb, st, &request->request) == -1) {
          err = true;
        }

        ss =+ selector_set_interest(key->s, socks5->client_fd, OP_WRITE);
        ss =+ selector_register(key->s, *fd, &socks5_handler, OP_NOOP, ATTACHMENT(key));
        if(ss != SELECTOR_SUCCESS){
            err = true;
            goto finally;
        }
        ret = REQUEST_WRITE;
    }
  }

  finally : return err ? ERROR : ret;
}



//////////////////////////////////////////////////////////////////////////////////////
// REQUEST_RESOLV
/////////////////////////////////////////////////////////////////////////////////////
unsigned int request_resolv_done(struct selector_key *key) {
  // TODO:
  return DONE;
}

//////////////////////////////////////////////////////////////////////////////////////
// REQUEST_CONNECTING
/////////////////////////////////////////////////////////////////////////////////////
void request_connecting_init(const unsigned state, struct selector_key *key) {

  connecting_data *conn = &ATTACHMENT(key)->server_data.connect;

  conn->client_fd = &ATTACHMENT(key)->client_fd;
  conn->final_server_fd = &ATTACHMENT(key)->final_server_fd;
  conn->wb = &ATTACHMENT(key)->write_buffer;

}

unsigned int request_connecting(struct selector_key *key) {
  
  unsigned int ret = REQUEST_WRITE;
  int err = false;
  socklen_t len = sizeof(err);

  connecting_data *conn = &ATTACHMENT(key)->server_data.connect;
  request_data *request = &ATTACHMENT(key)->client_data.request;
  enum socks_response_status response_st;

  if(getsockopt(key->fd, SOL_SOCKET, SO_ERROR, &err, &len) < 0){
    err = true;
    goto finally;
  }else{
    if(err == 0){
      //se conecto
      response_st = status_succeeded;
      ATTACHMENT(key)->status = response_st;
      *conn->final_server_fd = key->fd;
      add_connection();
      log_conn(ATTACHMENT(key), ATTACHMENT(key)->status);
    }else{
      //TODO: si hay mas ip, pruebo con otra :

      //Si no hay mas ip:
      response_st = errno_to_socks(err);
      ATTACHMENT(key)->status = response_st;
      log_conn(ATTACHMENT(key), ATTACHMENT(key)->status);
    }
  }

  if(request_marshall(request->wb, response_st, &request->request) == -1){
        err = true;
        goto finally;
      }

  selector_status ss = 0;
  ss += selector_set_interest(key->s, *conn->client_fd, OP_WRITE);
  ss += selector_set_interest(key->s, key->fd , OP_NOOP);

finally : return err ? ERROR : ret;

}


//////////////////////////////////////////////////////////////////////////////////////
// REQUEST_READ
/////////////////////////////////////////////////////////////////////////////////////
unsigned int request_write(struct selector_key *key) { 

  request_data *data = &ATTACHMENT(key)->client_data.request;
  //TODO BORRAR
  socks5 *aux = ATTACHMENT(key);
  buffer *buff = data->wb;
  size_t size;
  ssize_t n;

  uint8_t *ptr = buffer_read_ptr(buff, &size);
  n = send(key->fd, ptr, size, MSG_NOSIGNAL);
  if(n < 0){
    return ERROR;
  }else{
    buffer_read_adv(buff, n);
    if(!buffer_can_read(buff)){
      if(aux->status != status_succeeded){
        return DONE;
      }
      if(SELECTOR_SUCCESS == selector_set_interest_key(key, OP_READ)){
        return COPY;
      }else{
        return ERROR;
      } 
    }else{
      return REQUEST_READ;
    }
  }
}
