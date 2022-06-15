#include "../include/client_handler.h"

// funcione privada:
void socks5_done(struct selector_key *key);
void socks5_free(void* socks);

//////////////////////////////////////////////////////////////////////
void client_passive_accept(struct selector_key *key) {

  //TODO: falta ver el tema de los errores
  int err = 0;

  struct sockaddr_storage clntAddr;
  socklen_t clntAddrLen = sizeof(clntAddr);

  // Wait for a client to connect
  const int client = accept(key->fd, (struct sockaddr *)&clntAddr, &clntAddrLen);
  if (client == -1) {
    err = 1;
    goto finally;
  }

  log_print(INFO, "Creando socks5: %d", client);

  // clntSock is connected to a client!
  if (selector_fd_set_nio(client) == -1) {
    err = 1;
    goto finally;
  }

  socks5 *socks = socks5_new(client, &clntAddr, clntAddrLen);
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
    socks5_free(key);
  }
}

////////////////////////////////////////////////////////////////////////
// accepted socksv5 client handler
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
  const int fds[] = {
          ATTACHMENT(key)->client_fd,
          ATTACHMENT(key)->final_server_fd,
  };

  for (int i = 0; i < 2; ++i) {
    if(fds[i] != -1){
      if(SELECTOR_SUCCESS != selector_unregister_fd(key->s, fds[i])){
        log_print(INFO, "abort 1");
        for (int j = 0; j < 1000; ++j) {
        }
        abort();
      }
    }
  }
  log_print(INFO, "se cerro el socket: %d \n", fds[0]);
  //metricas de cantidad de conexiones
}

void socks5_close(struct selector_key *key) {
  close(key->fd);
  struct socks5 *newSocks = ATTACHMENT(key);
  if(newSocks->toFree > 0){
    socks5_free(key->data);
  }else{
    newSocks->toFree ++;
  }
}

void socks5_free(void* socks){
  free(socks);
}

