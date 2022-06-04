#include "../include/socks5.h"

// handler de cada estado de socks5
const struct state_definition socks_state_definition[] = {
    {
        .state = CONNECTING,
        .on_arrival = connecting_init,
        .on_departure = connecting_close,
        .on_read_ready = connecting_read,
        .on_write_ready = connecting_write,

    },
    {
        .state = COPY,
        .on_arrival = copy_init,
        .on_departure = copy_close,
        .on_read_ready = copy_read,
        .on_write_ready = copy_write,
    },
    {
        .state = DONE,
    },
    {
        .state = ERROR,
    }};

/////////////////////////////////////////////////////////////////////////
/*                                                                     */
/////////////////////////////////////////////////////////////////////////
struct socks5 *socks5_new(const int client) {

  struct socks5 *sockState = malloc(sizeof(struct socks5));
  if (sockState == NULL) {
    log(LOG_ERROR, "Error: Initizalizing null Socks5\n");
  }

  // inicializamos la maquina de estados
  sockState->stm.current = &socks_state_definition[0];
  sockState->stm.max_state = ERROR;
  sockState->stm.states = socks_state_definition;
  sockState->stm.initial = HELLO_READ;
  stm_init(&(sockState->stm));

  // creamos los buffer de lectura y escritura

  // TODO: cambiar
  buffer_init(&(sockState->write_buffer), BUFFERSIZE + 1,
              malloc(BUFFERSIZE + 1));
  buffer_init(&(sockState->read_buffer), BUFFERSIZE + 1,
              malloc(BUFFERSIZE + 1));

  // Intialize the client_fd and the server_fd
  sockState->client_fd = client;
  sockState->origin_fd = sockState->sel_origin_fd = -1;

  //   sockState->reply_type = -1;
  // 1 -> se puede borrar
  sockState->references = 1;
  //   sockState->username = NULL;
  memset(&sockState->server_info, 0, sizeof(sockState->server_info));

  return sockState;
}
