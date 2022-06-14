#include "../include/socks5.h"

// handler de cada estado de socks5
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
/*                                                                     */
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
