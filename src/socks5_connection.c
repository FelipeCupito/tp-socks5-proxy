#include "../include/socks5_connection.h"
/*
    Respuesta de conexion:

          o Versión del protocolo VER: X'05'
          o Campo de respuesta del REP:
             o X'00' succeeded
             o X'01' falla general del servidor SOCKS
             o X'02' Conexión no permitida por el conjunto de reglas
             o X'03' Red inalcanzable
             o X'04' Host inalcanzable
             o X'05' Conexión rechazada
             o X'06' TTL vencido
             o X'07' Comando no soportado
             o X'08' Tipo de dirección no compatible
             o X'09' a X'FF' sin asignar
          o RSV RESERVADO
          o Tipo de dirección ATYP de la siguiente dirección
             o Dirección IP V4: X'01'
             o NOMBRE DE DOMINIO: X'03'
             o Dirección IP V6: X'04'
          o BND.ADDR dirección vinculada al servidor
          o BND.PORT puerto vinculado al servidor en orden de octetos de red
*/

////////////////////////////////////////////////////////////////////////
// 
////////////////////////////////////////////////////////////////////////

void connecting_init(const unsigned state, struct selector_key *key) {

  // borrar: (esto va en el estado anterior) --->
  request_data *request = ATTACHMENT(key)->client_data.request;
  request->ipv4_size = 0;
  request->ipv4_addr[0][0] = "127.0.0.1";
  request->ipv6_size = -1; // no hay ip v6
  request->port[0] = 9090;
  request->addrType = 0x01;
  //<----

  connecting_data *conn = ATTACHMENT(key)->server_data.conn;
  conn->client_fd = ATTACHMENT(key)->client_fd;
  conn->final_server_fd = ATTACHMENT(key)->final_server_fd;
  conn->wb = ATTACHMENT(key)->write_buffer;

  // descomentar cuando se borre lo de arriba
  // request_data* request = ATTACHMENT(key)->client_data.request;

  // creo socket socket v4 o v6
  if (request->addrType == IPv4) {
    conn->final_server_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    conn->ip_index = request->ipv4_size;

  } else if (request->addrType == IPv6) {
    conn->final_server_fd = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
    // setsockopt(server, IPPROTO_IPV6, IPV6_V6ONLY, &(int){1}, sizeof(int));
    conn->ip_index = request->ipv6_size;

  } else {/* ERROR */ }

  if (conn->final_server_fd == -1) { /* ERROR en socket*/}
  if (conn->ip_index == -1) { /* ERROR nuestro*/ }
  
  if(selector_fd_set_nio(conn->final_server_fd) == -1){/*ERROR*/}

}

unsigned connecting_write(struct selector_key *key) {
    
    /* NOTAS:
        conn->ip_index: empieza con el maximo y decrementa uno hasta llegar a cero
    */

  int conn_status = -1;
  unsigned ret = CONNECTING;

  request_data *request = ATTACHMENT(key)->client_data.request;
  connecting_data *conn = ATTACHMENT(key)->server_data.conn;
  struct sockaddr_storage* sin =(struct sockaddr_storage*) &ATTACHMENT(key)->final_server_addr;

  //intento conectarme a todas las ip a la vez
  while( conn_status < 0 && conn->ip_index >= 0){
    
    //inicilizo sockaddr_storage
    if (request->addrType == IPv4) {
      sin->ss_family = AF_INET;
      memcpy(SIN_ADDR(sin), request->ipv4_addrs[conn->ip_index], IP_V4_ADDR_SIZE);
      memcpy(SIN_PORT(sin), conn->port, PORT_SIZE);

    } else if ( request->addrType == IPv6){
      sin->ss_family = AF_INET6;
      memcpy(SIN_ADDR6(sin), request->ipv6_addrs[conn->ip_index], IP_V6_ADDR_SIZE); //
      memcpy(SIN_PORT6(sin), conn->port, PORT_SIZE);
    }
    conn->ip_index --;
    socklen_t addrlen = sizeof(sin);

    //intento conectarme
    conn_status = connect(conn->final_server_fd,(struct sockaddr*)sin, addrlen);

    if(conn_status == -1){
        if (errno == EINPROGRESS) {
            //se esta conectando salgo del while
            break;
        }else{
            conn->ip_index--;
        }
    }else{ /*no se si es un error*/ }

  }//fin while




  selector_status ss = SELECTOR_SUCCESS;

  if (conn_status == -1) {
    switch (errno) {
    case EINPROGRESS:
      ss = selector_register(key->s, conn->final_server_fd, &socks5_handler,
                             OP_WRITE, ATTACHMENT(key));
      ss = selector_set_interess(key->s, socks5->client_fd, OP_NOOP);
      if (ss != SELECTOR_SUCCESS) {
        // TODO:ERROR
        return;
      }
      //
      d->substate = CONN_SUB_CHECK_ORIGIN;
      break;
    default:
      // si tengo otra ip para probar todo bien, sigo intentado
      // si no hay mas ip error, no me puedo conectar
    }
  }
  if (conn_status > 0) {
    ss = selector_register(key->s, s->sel_origin_fd, &socks5_handler,
                           OP_READ | OP_WRITE, ATTACHMENT(key));
    if (ss != SELECTOR_SUCCESS) {
      // ERROR
    }
  }
}

void connecting_read(const unsigned state, struct selector_key *key) {
  // TODO: hacer
}
