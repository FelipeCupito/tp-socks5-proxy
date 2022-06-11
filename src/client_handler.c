#include "../include/client_handler.h"

// funcione privada:
void socks5_done(struct selector_key *key);
static void socks5_destroy(struct socks5 *s);


//////////////////////////////////////////////////////////////////////
void client_passive_accept(struct selector_key *key) {

  //TODO: falta ver el tema de los errores
  
  struct sockaddr_storage clntAddr;
  socklen_t clntAddrLen = sizeof(clntAddr);

  // Wait for a client to connect
  const int client = accept(key->fd, (struct sockaddr *)&clntAddr, &clntAddrLen);
  if (client == -1) {
    goto fail;
  }

  log_print(INFO, "acepto al conecion(1)\n");

  // clntSock is connected to a client!
  if (selector_fd_set_nio(client) == -1) {
    goto fail;
  }

  socks5 *socks = socks5_new(client, &clntAddr, clntAddrLen);

  if (SELECTOR_SUCCESS !=
      selector_register(key->s, client, &socks5_handler, OP_READ, socks)) {
    log_print(LOG_ERROR, "no se registro en el select");
    goto fail;
  }

fail:
  if (client == -1) {
    close(client);
    // TODO: borrar sock5
  }
  // close(key->fd);
}

////////////////////////////////////////////////////////////////////////
// accepted socksv5 client handler
////////////////////////////////////////////////////////////////////////
void socks5_read(struct selector_key *key) {
  struct state_machine *stm = &ATTACHMENT(key)->stm;
  const enum socks_state st = stm_handler_read(stm, key);

  if (ERROR == st || DONE == st) {
    socks5_done(key);
    log_print(INFO, "client_read fallo");
    // TODO: hacer un close de socks5
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

void socks5_close(struct selector_key *key) {
  close(key->fd);
  socks5_destroy(ATTACHMENT(key));
}

void socks5_done(struct selector_key *key) {


  // metrics
 
}

static void socks5_destroy(struct socks5 *s) {
  
  //TODO

}