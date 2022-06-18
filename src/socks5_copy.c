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


fd_interest copy_determine_interests(fd_selector s, copy_data *data) {
  
  fd_interest ret = OP_NOOP;

  if ((data->interest & OP_READ) && buffer_can_write(data->rb)) {
    ret |= OP_READ;
  }
  if ((data->interest & OP_WRITE) && buffer_can_read(data->wb)) {
    ret |= OP_WRITE;
  }
  //data->interest = ret;

  if (SELECTOR_SUCCESS != selector_set_interest(s, *data->fd, ret)) {
    return -1;
  }
  return ret;
}

//devuelve copy_data correcto
copy_data* get_copy_data(struct selector_key *key, int* is_client){
  copy_data *client_copy = &ATTACHMENT(key)->client_data.copy;
  
  if(key->fd == *client_copy->fd){
    //es el cliente
    *is_client = true;
    return client_copy;
  }

  //es el final_server
  *is_client = false;
  client_copy = client_copy->other_copy;
  return client_copy;
}

unsigned int copy_read(struct selector_key *key) {
  
  unsigned int ret = COPY;
  int is_client;
  copy_data* data = get_copy_data(key, &is_client);

  buffer *buff = data->rb;
  size_t size; 
  ssize_t n;

  uint8_t* ptr = buffer_write_ptr(buff, &size);  
  
  n = recv(key->fd, ptr, size, 0);
  if (n > 0) {
    
    buffer_write_adv(buff, n);
    if(is_client){
      add_received_bytes(n);
      //TODO: -> pop3
      pop3_sniff(, ptr, n);
    }
  } else {
    ATTACHMENT(key)->status = status_close;
    shutdown(*data->fd, SHUT_RD);
    data->interest &= ~OP_READ;
    if (*data->other_copy->fd != -1) {
      shutdown(*data->other_copy->fd, SHUT_WR);
      data->other_copy->interest &= ~OP_WRITE;
    }
  }

  //TODO: si devulve -1 ->error, sino ver que hago con lo que devulvo
  copy_determine_interests(key->s, data);
  copy_determine_interests(key->s, data->other_copy);

  if (data->interest == OP_NOOP) {
    ret = DONE;
  }
  return ret;
}

static void pop3_sniff(struct pop3_sniffer *s, uint8_t *ptr, ssize_t size){
    if(!pop3_is_parsing(s)){
        pop3_sniffer_init(s);
    }
    if(!pop3_is_done(s)){
        size_t count;
        uint8_t *pop3_ptr = buffer_write_ptr(&s -> buffer,&count);
        // Pierdo info :/
        if((unsigned) size <= count){
            memcpy(pop3_ptr,ptr,size);
            buffer_write_adv(&s -> buffer,size);
        }
        else{
            memcpy(pop3_ptr,ptr,count);
            buffer_write_adv(&s -> buffer,count);
        }
        pop3_consume(s,&ATTACHMENT(key) -> socks_info);
    }
}

unsigned copy_write(struct selector_key *key) {
  
  unsigned int ret = COPY;
  int is_client;
  copy_data *data = get_copy_data(key, &is_client);
  

  buffer* buff = data->wb;
  size_t size; 
  ssize_t n;

  uint8_t* ptr = buffer_read_ptr(buff, &size);  
  
  n = send(key->fd, ptr, size, MSG_NOSIGNAL);
  if (n >= 0) {
    buffer_read_adv(buff, n);
    if(is_client){
      add_sent_byte(n);
      //TODO: -> pop2
    }
  } else {
    ATTACHMENT(key)->status = status_close;
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


