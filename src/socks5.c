#include "../include/socks5.h"



//funciones privadas:
void socks5_passive_accept(struct selector_key *key);

void socks5_read(struct selector_key *key);
void socks5_write(struct selector_key *key);
void socks5_block(struct selector_key *key);
void socks5_close(struct selector_key *key);

/////////////////////////////////////////////////////////////////////////
// FD HANDLER
/////////////////////////////////////////////////////////////////////////
const struct fd_handler socks5_passive_handler = {
    .handle_read = socks5_passive_accept,
    .handle_write = NULL,
    .handle_close = NULL,
};

const struct fd_handler socks5_handler = {
    .handle_read = socks5_read,
    .handle_write = socks5_write,
    .handle_close = socks5_close,
    .handle_block = socks5_block,
};

/////////////////////////////////////////////////////////////////////////
// ESTADOS DE SOCKS5
/////////////////////////////////////////////////////////////////////////
const struct state_definition socks_state_definition[] = {
    {
      .state = HELLO_READ,
      .on_arrival = hello_read_init,
      .on_read_ready = hello_read,
    },
    {
      .state = HELLO_WRITE,
      .on_write_ready = hello_write,
    },
    {
      .state = AUTH_READ,
      .on_arrival = auth_read_init,
      .on_read_ready = auth_read,
    },
    {
      .state = AUTH_WRITE,
      .on_write_ready = auth_write,
    },
    {
      .state = REQUEST_READ,
      .on_arrival = request_init,
      .on_read_ready = request_read,
    },
    {
      .state = REQUEST_RESOLV,
      .on_block_ready = request_resolv_done,
    },
    {
      .state = REQUEST_CONNECTING,
      .on_arrival = request_connecting_init,
      .on_write_ready = request_connecting,
    },
    {
      .state = REQUEST_WRITE,
      .on_write_ready = request_write,
    },
    {
      .state = COPY,
      .on_arrival = copy_init,
      .on_read_ready = copy_read,
      .on_write_ready = copy_write,
    },
    {
      .state = DONE,
    },
    {
      .state = ERROR,
    }
};

/////////////////////////////////////////////////////////////////////////
/*                                                                   */
/////////////////////////////////////////////////////////////////////////
struct socks5 *socks5_new(const int client, struct sockaddr_storage* clntAddr, socklen_t clntAddrLen) {

  struct socks5 *newSocks = malloc(sizeof(struct socks5));
  
  if (newSocks == NULL) {
    return NULL;
  }

  //smt:
  newSocks->stm.current = &socks_state_definition[0];
  newSocks->stm.max_state = ERROR;
  newSocks->stm.states = socks_state_definition;
  newSocks->stm.initial = HELLO_READ;
  stm_init(&(newSocks->stm));

  //init buffers
  buffer_init(&(newSocks->write_buffer), BUFFER_SIZE + 1, newSocks->raw_buff_a);
  buffer_init(&(newSocks->read_buffer), BUFFER_SIZE + 1, newSocks->raw_buff_b);

  // init fds
  newSocks->client_fd = client;
  newSocks->final_server_fd = -1;

  //info del socket cliente
  memcpy(&newSocks->client_addr, clntAddr, clntAddrLen);
  newSocks->client_addr_len = clntAddrLen;

  //si esta en 1 se puede liberar la memoria
  newSocks->toFree = 0;

  return newSocks;
}


/////////////////////////////////////////////////////////////////////////
/*  PASSIVE SOCKS                                                      */
/////////////////////////////////////////////////////////////////////////
// funcione privada:
void socks5_done(struct selector_key *key);
void socks5_free(struct socks5 *socks5);
//socks5 handler
void socks5_read(struct selector_key *key);
void socks5_write(struct selector_key *key);
void socks5_block(struct selector_key *key);
void socks5_close(struct selector_key *key);

void socks5_passive_accept(struct selector_key *key) {

  int err = 0;
  struct sockaddr_storage clntAddr;
  socklen_t clntAddrLen = sizeof(clntAddr);
  socks5 *socks = NULL;

  // Wait for a client to connect
  const int client = accept(key->fd, (struct sockaddr *)&clntAddr, &clntAddrLen);
  if (client == -1) {
    err = 1;
    goto finally;
  }

  // clntSock is connected to a client!
  if (selector_fd_set_nio(client) == -1) {
    err = 1;
    goto finally;
  }

  socks = socks5_new(client, &clntAddr, clntAddrLen);
  if(socks == NULL){
    err = 1;
    goto finally;
  }


  if (SELECTOR_SUCCESS !=
      selector_register(key->s, client, &socks5_handler, OP_READ, socks)) {
    err = 1;
    goto finally;
  }

finally:
  if(err == 1){
    close(client);
    free(socks);
  }
}

////////////////////////////////////////////////////////////////////////
// accepted socksv5 handler
////////////////////////////////////////////////////////////////////////
void socks5_read(struct selector_key *key) {
  struct state_machine *stm = &ATTACHMENT(key)->stm;
   const enum socks_state st = stm_handler_read(stm, key);

  if (ERROR == st || DONE == st) {
    socks5_done(key);
  }
}

void socks5_write(struct selector_key *key) {
  struct state_machine *stm = &ATTACHMENT(key)->stm;
  const enum socks_state st = stm_handler_write(stm, key);

  if (ERROR == st || DONE == st) {
    socks5_done(key);
  }
}

void socks5_block(struct selector_key *key) {
  struct state_machine *stm = &ATTACHMENT(key)->stm;
  const enum socks_state st = stm_handler_block(stm, key);

  if (ERROR == st || DONE == st) {
    socks5_done(key);
  }
}

void socks5_done(struct selector_key *key) {

  if (ATTACHMENT(key)->status == status_close){
    log_conn(ATTACHMENT(key), ATTACHMENT(key)->status);
    end_connection();
  }

  const int fds[] = {
          ATTACHMENT(key)->client_fd,
          ATTACHMENT(key)->final_server_fd,
  };

  for (int i = 0; i < 2; ++i) {
    if (fds[i] != -1) {
      selector_unregister_fd(key->s, fds[i]);
    }
    close(fds[i]);
  }
}


void socks5_close(struct selector_key *key) {
  struct socks5 *socks = ATTACHMENT(key);
  if(socks->status != status_close ){
    socks5_free(socks);
  }else if(socks->toFree > 0){
    socks5_free(key->data);
  }else{
    socks->toFree ++;
  }
}

void socks5_free(struct socks5 *socks5){
  if(socks5->server_resolution != NULL){
    freeaddrinfo(socks5->server_resolution);
  }
  free(socks5);
}

