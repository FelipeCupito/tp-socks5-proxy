#include "../include/socks5_copy.h"

////////////////////////////////////////////////////////////////////////
// COPY
////////////////////////////////////////////////////////////////////////
void copy_init(const unsigned state, struct selector_key *key) {
  copy_data* copy = &ATTACHMENT(key)->client_data.copy;

  //client_socket
  copy->fd = &ATTACHMENT(key)->client_fd;
  copy->rb = &ATTACHMENT(key)->read_buffer;
  copy->wb = &ATTACHMENT(key)->write_buffer;
  copy->interest = OP_READ | OP_WRITE;
  copy->other_copy = &ATTACHMENT(key)->server_data.copy;

  //server_socket
  copy = &ATTACHMENT(key)->server_data.copy;
  copy->fd = &ATTACHMENT(key)->final_server_fd;
  copy->rb = &ATTACHMENT(key)->write_buffer;
  copy->wb = &ATTACHMENT(key)->read_buffer;
  copy->interest = OP_READ | OP_WRITE;
  copy->other_copy = &ATTACHMENT(key)->client_data.copy;
}

void copy_close(const unsigned state, struct selector_key *key) {
  /*TODO. */
}


fd_interest copy_determine_interests(fd_selector s, copy_data *data) {
  
  fd_interest ret = OP_NOOP;

  if ((data->interest & OP_READ) && buffer_can_write(data->rb)) {
    ret |= OP_READ;
  }

  if ((data->interest & OP_WRITE) && buffer_can_read(data->wb)) {
    ret |= OP_WRITE;
  }

  if (SELECTOR_SUCCESS != selector_set_interest(s, *data->fd, ret)) {
    log_print(LOG_ERROR,"Could not set interest of %d for %d\n", ret, data->fd);
    //TODO: exit
  }
  return ret;
}

//devuelve copy_data correcto
copy_data* get_copy_data(struct selector_key *key){
  copy_data *client_copy = &ATTACHMENT(key)->client_data.copy;
  
  if(key->fd == *client_copy->fd){
    //es el cliente
    int i = 1;//TODO: borrar
    return client_copy;
  }

  //es el final_server 
  int j = 0;//TODO: borrar
  client_copy = client_copy->other_copy;
  return client_copy;
}

unsigned int copy_read(struct selector_key *key) {
  copy_data* data = get_copy_data(key);
  unsigned int ret = COPY;
  unsigned int err = false;

  buffer *buff = data->rb;
  size_t size; 
  ssize_t n;

  uint8_t* ptr = buffer_write_ptr(buff, &size);  
  
  n = recv(key->fd, ptr, size, 0);
  if (n > 0) {

    //TODO: metricas
    //TODO: si es cliente -> pop2
    buffer_write_adv(buff, n);
  } else {
   
    shutdown(*data->fd, SHUT_RD);
    data->interest &= ~OP_READ;
    if (*data->other_copy->fd != -1) {
      shutdown(*data->other_copy->fd, SHUT_WR);
      data->other_copy->interest &= ~OP_WRITE;
    }
  }

  copy_determine_interests(key->s, data);
  copy_determine_interests(key->s, data->other_copy);

  if (data->interest == OP_NOOP) {
    ret = DONE;
  }
  return ret;
}

unsigned copy_write(struct selector_key *key) {
  copy_data *data = get_copy_data(key);
  unsigned int ret = COPY;
  unsigned int err = false;

  buffer* buff = data->wb;
  size_t size; 
  ssize_t n;

  uint8_t* ptr = buffer_read_ptr(buff, &size);  
  
  n = send(key->fd, ptr, size, MSG_NOSIGNAL);
  if (n > 0) {

    //TODO: metricas
    //TODO: si es cliente -> pop2
    buffer_read_adv(buff, n);
  } else {
   
    shutdown(*data->fd, SHUT_WR);
    data->interest &= ~OP_WRITE;
    if (*data->other_copy->fd != -1) {
      shutdown(*data->other_copy->fd, SHUT_RD);
      data->other_copy->interest &= ~OP_READ;
    }
  }

  copy_determine_interests(key->s, data);
  copy_determine_interests(key->s, data->other_copy);

  if (data->interest == OP_NOOP) {
    ret = DONE;
  }
  return ret;
}


