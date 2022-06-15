#include "../include/conn_logger.h"


#define DATE_SIZE 30
#define DATE_FORMAT "%d/%m/%Y - %X"
//#define DATE_FORMAT "%d/%m/%Y - %I:%M%p"
#define INET_ADDRSTRLEN 16
#define INET6_ADDRSTRLEN 46

char* status_str[] = {"CONNECTED", "DISCONNECTED"};

void get_date(char *date, int date_size);
void get_ip(struct sockaddr_storage addr, char *ip, int ip_size);
int get_port(struct sockaddr_storage addr);
char *get_status(STATUS status);

void log_conn( void* data, STATUS status) {
  // TODO: USUARIO/CONTRASEÑA

  struct socks5* socks = (struct socks5*) data;

  // guardo la fecha actual
  char date[DATE_SIZE];
  get_date(date, DATE_SIZE);

  // ip
  char ip[INET6_ADDRSTRLEN];
  get_ip(socks->client_addr, ip, INET6_ADDRSTRLEN);
  char ip_dest[INET6_ADDRSTRLEN];
  get_ip(socks->final_server_addr, ip_dest, INET6_ADDRSTRLEN);

  // ports
  int port = get_port(socks->client_addr);
  int port_dest = get_port(socks->final_server_addr);

  log_print(LOG, "[%s]\t%s:%d ---> %s:%d\t%s", date, ip, port, ip_dest,
            port_dest, get_status(status));
}



/// funciones privadas
void get_date(char *date, int date_size) {
  time_t timer = time(NULL);
  struct tm *tm = localtime(&timer);
  strftime(date, date_size, "%d/%m/%Y - %X", tm);
}

void get_ip(struct sockaddr_storage addr, char *ip, int ip_size) {
  if (addr.ss_family == AF_INET) {
    inet_ntop(addr.ss_family, &(((struct sockaddr_in *)&addr)->sin_addr), ip,
              ip_size);

  } else {
    inet_ntop(addr.ss_family, &(((struct sockaddr_in6 *)&addr)->sin6_addr), ip,
              ip_size);
  }
}

int get_port(struct sockaddr_storage addr) {
  if (addr.ss_family == AF_INET) {
    return ntohs(((struct sockaddr_in *)&addr)->sin_port);
  } else {
    return ntohs(((struct sockaddr_in6 *)&addr)->sin6_port);
  }
}

char *get_status(STATUS status) { return status_str[status]; }
