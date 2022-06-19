#include "../include/socks5_request.h"

// privadas:
unsigned int request_connect(struct selector_key *key, request_data *request);
unsigned request_process(struct selector_key *key, request_data *request);
void *request_resolv_blocking(void *data);
int next_ip(struct socks5 *socks5, request_data *request);

///////////////////////////////////////////
// REQUEST_READ
//////////////////////////////////////////
void request_init(const unsigned state, struct selector_key *key) {

  request_data *data = &ATTACHMENT(key)->client_data.request;

  // parser
  data->rb = &ATTACHMENT(key)->read_buffer;
  data->wb = &ATTACHMENT(key)->write_buffer;
  data->parser.request = &data->request;
  data->status =  &ATTACHMENT(key)->status;
  request_parser_init(&data->parser);

  // fd
  data->client_fd = &ATTACHMENT(key)->client_fd;
  data->final_server_fd = &ATTACHMENT(key)->final_server_fd;

  // addr
  data->final_server_addr = &ATTACHMENT(key)->final_server_addr;
  data->final_server_len = &ATTACHMENT(key)->final_server_len;
  data->server_domain = &ATTACHMENT(key)->server_domain;
}

unsigned int request_read(struct selector_key *key) {
  request_data *data = &ATTACHMENT(key)->client_data.request;
  unsigned int ret = REQUEST_READ;
  bool err = false;

  buffer *buff = data->rb;
  size_t size;
  ssize_t n;

  uint8_t *ptr = buffer_write_ptr(buff, &size);
  n = recv(key->fd, ptr, size, 0);
  if (n < 0) {
    err = true;
  } else {
    buffer_write_adv(buff, n);
    request_state st = request_consume(buff, &data->parser, &err);
    //TODO: request_is_done el ultimo parametro no tiene que ser &err?????
    if (request_is_done(st, 0)) {
      ret = request_process(key, data);
    }
  }
  return err ? ERROR : ret;
}

// Privadas:
unsigned request_process(struct selector_key *key, request_data *request) {
  unsigned int ret;
  pthread_t tid;
  struct selector_key *k;

  struct sockaddr_in addr4;
  struct sockaddr_in6 addr6;
  ssize_t len;

  if (request->request.cmd == socks_req_cmd_connect){
    switch (request->request.dst_addr_type) {
      case socks_req_addrtype_ipv4:
        addr4 = request->request.dst_addr.ipv4;
        addr4.sin_family = AF_INET;
        addr4.sin_port = request->request.dst_port;

        len = sizeof(request->request.dst_addr.ipv4);
        *request->final_server_len = len;
        memcpy(request->final_server_addr, &addr4, len);

        ret = request_connect(key, request);
        break;

      case socks_req_addrtype_ipv6:
        addr6 = request->request.dst_addr.ipv6;
        addr6.sin6_family = AF_INET6;
        addr6.sin6_port = request->request.dst_port;

        len = sizeof(request->request.dst_addr.ipv6);
        *request->final_server_len = len;
        memcpy(request->final_server_addr, &addr6, len);

        ret = request_connect(key, request);
        break;

      case socks_req_addrtype_domain:
        k = malloc(sizeof(*key));
        if (k == NULL) {
          ret = REQUEST_WRITE;
          *request->status = status_general_SOCKS_server_failure;
          selector_set_interest_key(key, OP_WRITE);
        } else {
          memcpy(k, key, sizeof(*k));
          if (-1 == pthread_create(&tid, 0, request_resolv_blocking, k)) {
            ret = REQUEST_WRITE;
            *request->status = status_general_SOCKS_server_failure;
            selector_set_interest_key(key, OP_WRITE);
          } else {
            ret = REQUEST_RESOLV;
            selector_set_interest_key(key, OP_NOOP);
          }
        }
        break;

      default:
        *request->status = status_address_type_not_supported;
        ret = REQUEST_WRITE;
        selector_set_interest_key(key, OP_WRITE);
        break;
    }
  } else {
    *request->status = status_command_not_supported;
    ret = REQUEST_WRITE;
    selector_set_interest_key(key, OP_WRITE);
  }

  return ret;
}

unsigned int request_connect(struct selector_key *key, request_data *request) {
  unsigned int ret = ERROR;
  bool err = false;
  int *fd = request->final_server_fd;
  selector_status ss;

  if (*fd != -1) {
    selector_unregister_fd(key->s, *fd);
    close(*fd);
  }

  // creo el socket
  *fd = socket(request->final_server_addr->ss_family, SOCK_STREAM, IPPROTO_TCP);
  if (*fd == -1) {
    err = true;
    goto finally;
  }

  // hago que fd sea no bloqueante
  if (selector_fd_set_nio(*fd) == -1) {
    err = true;
    goto finally;
  }

  struct sockaddr_storage *sin = (struct sockaddr_storage*) request->final_server_addr;
  socklen_t sinLen = *request->final_server_len;
  if (connect(*fd, (struct sockaddr *)sin, sinLen) == -1) {
    switch (errno) {
      case EINPROGRESS:
        ss = selector_register(key->s, *fd, &socks5_handler, OP_WRITE,ATTACHMENT(key));
        if (ss != SELECTOR_SUCCESS) {
          err = true;
          goto finally;
        }
        ss = selector_set_interest(key->s, *request->client_fd, OP_NOOP);
        if (ss != SELECTOR_SUCCESS) {
          err = true;
          goto finally;
        }
        ret = REQUEST_CONNECTING;
        break;
    default:
      // hubo un error:
      *request->status = errno_to_socks(errno);
      if(next_ip(ATTACHMENT(key),request) != -1){
        log_conn(ATTACHMENT(key), *request->status);
        ret = request_connect(key, request);
      }else {
        // si no hay mas ip, comunico el error
        log_conn(ATTACHMENT(key), *request->status);
        if (request_marshalll(request->wb, *request->status, &request->request) == -1) {
          err = true;
        }
        if (SELECTOR_SUCCESS != selector_set_interest(key->s, *request->client_fd, OP_WRITE)){
          err = true;
          goto finally;
        }
        ret = REQUEST_WRITE;
      }
    }
  }

finally:
  return err ? ERROR : ret;
}

//////////////////////////////////////////////////////////////////////////////////////
// REQUEST_RESOLV
/////////////////////////////////////////////////////////////////////////////////////
unsigned int request_resolv_done(struct selector_key *key) {
  request_data *request = &ATTACHMENT(key)->client_data.request;
  struct socks5 *socks5 = ATTACHMENT(key);
 
  if (ATTACHMENT(key)->server_resolution == 0) {
    *request->status = status_general_SOCKS_server_failure;
  } else {
    *request->server_domain = socks5->server_resolution->ai_family;
    *request->final_server_len = socks5->server_resolution->ai_addrlen;
    memcpy(request->final_server_addr, socks5->server_resolution->ai_addr,
           socks5->server_resolution->ai_addrlen);

  }
  return request_connect(key, request);
}

int next_ip(struct socks5 *socks5, request_data *request){
  if(socks5->server_resolution == NULL || socks5->current_server_resolution->ai_next == NULL ) {

    return -1;
  }

  //log_conn de la conexion
  //log_conn(socks5, socks5->status);

  socks5->current_server_resolution = socks5->current_server_resolution->ai_next;
  *request->server_domain = socks5->server_resolution->ai_family;
  *request->final_server_len = socks5->server_resolution->ai_addrlen;
  memcpy(request->final_server_addr, socks5->server_resolution->ai_addr,
          socks5->server_resolution->ai_addrlen);
  return 0;
}

void* request_resolv_blocking(void *data) {
  struct selector_key *key = (struct selector_key *)data;
  request_data *request = &ATTACHMENT(key)->client_data.request;

  pthread_detach(pthread_self());
  ATTACHMENT(key)->server_resolution = 0;
  struct addrinfo hints = {
      .ai_family = AF_UNSPEC,     // Allow IPv4 or IPv6
      .ai_socktype = SOCK_STREAM, // Datagram socket
      .ai_flags = AI_PASSIVE,     // For wildcard IP address
      .ai_protocol = 0,           // Any protocol
      .ai_canonname = NULL,
      .ai_addr = NULL,
      .ai_next = NULL,
  };

  char buff[7];
  snprintf(buff, sizeof(buff), "%d",ntohs(request->request.dst_port));

  getaddrinfo(request->request.dst_addr.fqdn, buff, &hints,
              &ATTACHMENT(key)->server_resolution);

  ATTACHMENT(key)->current_server_resolution = ATTACHMENT(key)->server_resolution;

  selector_notify_block(key->s, key->fd);
  free(data);

  return 0;
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

  int err;
  socklen_t len = sizeof(err);

  connecting_data *conn = &ATTACHMENT(key)->server_data.connect;
  request_data *request = &ATTACHMENT(key)->client_data.request;

  if (getsockopt(key->fd, SOL_SOCKET, SO_ERROR, &err, &len) < 0) {
    *request->status = status_general_SOCKS_server_failure;
  } else {
    if (err == 0) {
      // se conecto
      *request->status = status_succeeded;
      *conn->final_server_fd = key->fd;
      add_connection();
      log_conn(ATTACHMENT(key), *request->status);
    } else {
      //no se puedo conexion, intento con otro ip.
      *request->status = errno_to_socks(err);
      if(next_ip(ATTACHMENT(key),request) != -1){
        return request_connect(key, request);
      }else {
      // Si no hay mas ip:
      log_conn(ATTACHMENT(key), *request->status);
      }
    }
  }

  if (request_marshalll(request->wb, *request->status, &request->request) == -1) {
    *request->status = status_general_SOCKS_server_failure;
    abort();
  }

  selector_status ss = 0;
  ss += selector_set_interest(key->s, *conn->client_fd, OP_WRITE);
  ss += selector_set_interest(key->s, key->fd, OP_NOOP);

  return ss == SELECTOR_SUCCESS ? REQUEST_WRITE : ERROR;
}

//////////////////////////////////////////////////////////////////////////////////////
// REQUEST_WRITE
/////////////////////////////////////////////////////////////////////////////////////
unsigned int request_write(struct selector_key *key) {

  request_data *data = &ATTACHMENT(key)->client_data.request;

  unsigned int ret = REQUEST_WRITE;
  buffer *buff = data->wb;
  size_t size;
  ssize_t n;

  uint8_t *ptr = buffer_read_ptr(buff, &size);
  n = send(key->fd, ptr, size, MSG_NOSIGNAL);
  if (n < 0) {
    ret = ERROR;
  } else {
    buffer_read_adv(buff, n);
    if (!buffer_can_read(buff)) {
      if (ATTACHMENT(key)->status == status_succeeded) {
        ret = COPY;
        selector_set_interest(key->s, *data->client_fd, OP_READ);
        selector_set_interest(key->s, *data->final_server_fd, OP_READ);
      } else {
        ret = DONE;
        selector_set_interest(key->s, *data->client_fd, OP_NOOP);
        if (*data->final_server_fd != -1) {
          selector_set_interest(key->s, *data->client_fd, OP_NOOP);
        }
      }
    }
  }
  return ret;
}
