#include "../include/socks5_request.h"

#define SIN_ADDR(sin) (void *)&(((struct sockaddr_in *)sin)->sin_addr)
#define SIN_PORT(sin) (void *)&(((struct sockaddr_in *)sin)->sin_port)

#define SIN_ADDR6(sin) (void *)&(((struct sockaddr_in6 *)sin)->sin6_addr)
#define SIN_PORT6(sin) (void *)&(((struct sockaddr_in6 *)sin)->sin6_port)

// privadas:
unsigned int request_connect(struct selector_key *key);

///////////////////////////////////////////
// REQUEST_READ
//////////////////////////////////////////
void request_init(const unsigned state, struct selector_key *key) {
    //TODO:
}

unsigned int request_read(struct selector_key *key) {

  unsigned int ret = REQUEST_READ;
  unsigned int err = 0;

  // TODO: Reemplazar los valores por los que realmente nos piden
  struct sockaddr_in sin;
  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = inet_addr("0.0.0.0");
  sin.sin_port = htons(9090);
  
  
  //inicializo valores
  socks5 *socks5= ATTACHMENT(key);
  memcpy(&socks5->final_server_addr, &sin, sizeof(sin));
  socks5->final_server_len = sizeof(struct sockaddr_storage);
  socks5->final_server_fd = -1;

  //inicio nueva conexion
  ret = request_connect(key);

  return err ? ERROR : ret;
}

unsigned int request_connect(struct selector_key *key) {
  unsigned int ret = REQUEST_CONNECTING;
  bool err = false;

  struct socks5 *socks5 = ATTACHMENT(key);
  int *fd = &socks5->final_server_fd;


  if (*fd != -1) {
    //no es la primera vez que intentamos conectarnos
    selector_unregister_fd(key->s, *fd);
    if (close(*fd) == -1) {
      err = true;
      goto finally;
    }
    // intentamos con otra IP
  }

  ret = REQUEST_CONNECTING;
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

//hago el connect
  struct sockaddr_storage *sin = (struct sockaddr_storage *)&socks5->final_server_addr;
  socklen_t addrlen = socks5->final_server_len;

  if (connect(*fd, (struct sockaddr *)sin, addrlen) == -1) {

    selector_status ss = SELECTOR_SUCCESS;
    switch (errno) {
      case EINPROGRESS:
        ss = selector_register(key->s, *fd, &socks5_handler, OP_WRITE, ATTACHMENT(key));
        if (ss != SELECTOR_SUCCESS) {
          err = true;
          goto finally;;
        }
        ss = selector_set_interest(key->s, socks5->client_fd , OP_NOOP);
        if (ss != SELECTOR_SUCCESS) {
          err = true;
          goto finally;;
        }
        break;
      default:
        //TODO: paso a otra ip o error
        printf("hola");
        err = true;
        goto finally;
    }
  }

  finally : return err ? ERROR : ret;
}


void request_read_close(const unsigned state, struct selector_key *key) {
  // TODO: close
}

//////////////////////////////////////////////////////////////////////////////////////
// REQUEST_RESOLV
/////////////////////////////////////////////////////////////////////////////////////
unsigned int request_resolv_done(struct selector_key *key) {
  // TODO:
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
  
  int error;
  socklen_t len = sizeof(error);
  connecting_data *conn = &ATTACHMENT(key)->server_data.connect;

  unsigned int ret = REQUEST_CONNECTING;
  
  if(getsockopt(key->fd, SOL_SOCKET, SO_ERROR, &error, &len) < 0){
    //TODO: fallo getsockopt
    //*conn->status = status_general_SOCKS_server_failure;
  }else{
    if(error == 0){
      //se conecto
      //metrics
      //*conn->status = status_succeeded;
      *conn->final_server_fd = key->fd;

    }else{
      //*conn->status = errno_to_socks(error);
      //TODO: si hay mas ip pruebo con otra
      //
    }
  }

  //request_marshall()

  selector_status ss = 0;
  ss |= selector_set_interest(key->s, *conn->client_fd, OP_WRITE);
  ss |= selector_set_interest(key->s, key->fd , OP_NOOP);


  return SELECTOR_SUCCESS == 0 ? REQUEST_WRITE : ERROR;
}

/*
enum socks_response_status errno_to_socks(const int e) {
  enum socks_response_status ret = status_general_SOCKS_server_failure;
  switch (e) {
    case 0:
      ret = status_succeeded;
      break;
    case ECONNREFUSED:
      ret = status_connection_refused;
      break;
    case EHOSTUNREACH:
      ret = status_host_unreachable;
      break;
    case ENETUNREACH:
      ret = status_network_unreachable;
      break;
    case ETIMEDOUT:
      ret = status_ttl_expired;
      break;
  }
  return ret;
}
*/

//////////////////////////////////////////////////////////////////////////////////////
// REQUEST_READ
/////////////////////////////////////////////////////////////////////////////////////
unsigned int request_write(struct selector_key *key) { 
  
  //TODO: por ahora
  fd_interest ret = OP_READ | OP_WRITE;
  connecting_data *conn = &ATTACHMENT(key)->server_data.connect;
  if (SELECTOR_SUCCESS != selector_set_interest(key->s, *conn->final_server_fd, ret)) {
    log_print(LOG_ERROR,"Could not set interest of %d for %d\n", ret, *conn->final_server_fd);
  }
  if (SELECTOR_SUCCESS != selector_set_interest(key->s, *conn->client_fd, ret)) {
    log_print(LOG_ERROR,"Could not set interest of %d for %d\n", ret, *conn->client_fd);
  }
  return COPY;
}
