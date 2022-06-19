#include "../include/mng_handler.h"

// funcione privada:
mng* mng_new(const int client, struct sockaddr_storage* clntAddr, socklen_t clntAddrLen) ;
void mng_passive_accept(struct selector_key *key);
void mng_done(struct selector_key *key);
//socks5 handler
void mng_read(struct selector_key *key);
void mng_write(struct selector_key *key);
void mng_block(struct selector_key *key);
void mng_close(struct selector_key *key);

/////////////////////////////////////////////////////////////////////////
// FD HANDLER
/////////////////////////////////////////////////////////////////////////
const struct fd_handler mng_passive_handler = {
        .handle_read = mng_passive_accept,
        .handle_write = NULL,
        .handle_close = NULL,
};

const struct fd_handler mng_handler = {
        .handle_read = mng_read,
        .handle_write = mng_write,
        .handle_close = mng_close,
        .handle_block = mng_block,
};

/////////////////////////////////////////////////////////////////////////
// ESTADOS DE SOCKS5
/////////////////////////////////////////////////////////////////////////
const struct state_definition mng_state_definition[] = {
        {
                .state = HELLO_READ,
                .on_arrival = mng_hello_read_init,
                .on_read_ready = mng_hello_read,
        },
        {
                .state = HELLO_WRITE,
                .on_write_ready = mng_hello_write,
        },
        {
                .state = REQUEST,
                .on_arrival = mng_request_init,
                .on_read_ready = mng_request,
        },
        {
                .state = REPLIES,
                .on_write_ready = mng_replies,
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
mng* mng_new(const int client, struct sockaddr_storage* clntAddr, socklen_t clntAddrLen) {

  struct mng *newMng = malloc(sizeof(struct mng));

  if (newMng == NULL) {
    return NULL;
  }

  //smt:
  newMng->stm.current = &mng_state_definition[0];
  newMng->stm.max_state = ERROR;
  newMng->stm.states = mng_state_definition;
  newMng->stm.initial = HELLO_READ;
  stm_init(&(newMng->stm));

  //init buffers
  buffer_init(&(newMng->write_buffer), BUFFER_SIZE + 1, newMng->raw_buff_a);
  buffer_init(&(newMng->read_buffer), BUFFER_SIZE + 1, newMng->raw_buff_b);

  // init fds
  newMng->client_fd = client;

  //info del socket cliente
  memcpy(&newMng->client_addr, clntAddr, clntAddrLen);
  newMng->client_addr_len = clntAddrLen;

  //si esta en 1 se puede liberar la memoria
  newMng->toFree = 0;

  return newMng;
}

/////////////////////////////////////////////////////////////////////////
/*  PASSIVE SOCKS                                                      */
/////////////////////////////////////////////////////////////////////////
void mng_passive_accept(struct selector_key *key) {

  int err = 0;
  struct sockaddr_storage clntAddr;
  socklen_t clntAddrLen = sizeof(clntAddr);
  struct mng *newMng = NULL;

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

  newMng = mng_new(client, &clntAddr, clntAddrLen);
  if(newMng == NULL){
    err = 1;
    goto finally;
  }


  if (SELECTOR_SUCCESS !=
      selector_register(key->s, client, &mng_handler, OP_READ, newMng)) {
    err = 1;
    goto finally;
  }

  finally:
  if(err == 1){
    close(client);
    free(newMng);
  }
}

////////////////////////////////////////////////////////////////////////
// accepted socksv5 handler
////////////////////////////////////////////////////////////////////////
void mng_read(struct selector_key *key) {
  struct state_machine *stm = &ATTACHMENT(key)->stm;
  const enum mng_state st = stm_handler_read(stm, key);

  if (ERROR == st || DONE == st) {
    mng_done(key);
  }
}

void mng_write(struct selector_key *key) {
  struct state_machine *stm = &ATTACHMENT(key)->stm;
  const enum mng_state st = stm_handler_write(stm, key);

  if (ERROR == st || DONE == st) {
    mng_done(key);
  }
}

void mng_block(struct selector_key *key) {
  struct state_machine *stm = &ATTACHMENT(key)->stm;
  const enum mng_state st = stm_handler_block(stm, key);

  if (ERROR == st || DONE == st) {
    mng_done(key);
  }
}

void mng_done(struct selector_key *key) {

  int fd = ATTACHMENT(key)->client_fd;
  selector_unregister_fd(key->s, fd);
  close(fd);

}

void mng_close(struct selector_key *key) {
  free(ATTACHMENT(key));
}

