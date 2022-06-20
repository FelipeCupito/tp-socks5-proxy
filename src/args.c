#include "../include/args.h"

//privadas:
static unsigned short port(const char *s);
static void user(char *s, struct users *user);
static void version(void);
static void usage(const char *progname);

void fill_sockaddr_v4(struct sockaddr_in* addr, int port, char* ip);
void fill_sockaddr_v6(struct sockaddr_in6* addr, int port, char* ip); 
int fill_ip_sockaddr(const char *ip, struct sockaddr_in* addrV4, struct sockaddr_in6* addrV6);
void fill_port_sockaddr(int port, struct sockaddr_in* addrV4, struct sockaddr_in6* addrV6);

void parse_args(const int argc, char **argv, config *configuration) {

  // default socks proxy
  fill_sockaddr_v4(&configuration->socksV4, DEFAULT_SOCKS_PORT, DEFAULT_SOCKS_ADDRESS_V4);
  configuration->socksV4_flag = true;
  fill_sockaddr_v6(&configuration->socksV6, DEFAULT_SOCKS_PORT, DEFAULT_SOCKS_ADDRESS_V6);
  configuration->socksV6_flag = true;

  // mng configuration
  fill_sockaddr_v4(&configuration->mngV4, DEFAULT_MNG_PORT, DEFAULT_MNG_ADDRESS_V4);
  configuration->mngV4_flag = true;
  fill_sockaddr_v6(&configuration->mngV6, DEFAULT_MNG_PORT, DEFAULT_MNG_ADDRESS_V6);
  //TODO: sino (unable to bind socket_v6)
  configuration->mngV6_flag = false;


  // guardo los argumentos
  int c;
  int ipv;
  int nusers = 0;

  while ((c = getopt(argc, argv, "hl:L:Np:P:u:v")) != -1) {

    switch (c) {
    case 'h':
      usage(argv[0]);
      break;
    case 'l':
      //ip socks
      ipv = fill_ip_sockaddr(optarg, &configuration->socksV4, &configuration->socksV6);
      if (ipv == -1) {
        log_print(LOG_ERROR, "invalid server ip addr: %s\n", optarg);
        goto finally;
      }
      ipv == 4 ? (configuration->socksV6_flag = false) : (configuration->socksV4_flag = false);
      break;
    case 'L':
      ipv = fill_ip_sockaddr(optarg, &configuration->mngV4, &configuration->mngV6);
      if (ipv == -1) {
        log_print(LOG_ERROR, "invalid Mng server ip addr: %s\n", optarg);
        goto finally;
      }
      ipv == 4 ? (configuration->mngV6_flag = false) : (configuration->mngV4_flag = false);
      break;
    case 'N':
      configuration->disectors_enabled = false;
      break;
    case 'p':
      fill_port_sockaddr(port(optarg), &configuration->socksV4, &configuration->socksV6);
      break;
    case 'P':
      fill_port_sockaddr(port(optarg), &configuration->mngV4, &configuration->mngV6);
      break;
    case 'u':
      if (nusers >= MAX_USERS) {
        log_print(LOG_ERROR, "maximum number of command line users reached: %d.\n",
            MAX_USERS);
      } else {
        user(optarg, configuration->users + nusers);
        nusers++;
        configuration->users_size = nusers;
      }
      break;
    case 'v':
      version();
      exit(0);
    default:
      log_print(LOG_ERROR, "unknown argument %d.\n", c);
    }
  }

  if(nusers > 0){
    configuration->auth_enabled = true;
  }

  if (optind < argc) {
    log_print(LOG_ERROR, "argument not accepted: ");
    while (optind < argc) {
      log_print(LOG_ERROR, "%s ", argv[optind++]);
    }
  }

finally:
  if (error_flag) {
    exit(1);
  }
}


////////////////////////////////////////////////////////
/*FUNCIONES PRIVADAS*/
///////////////////////////////////////////////////////

static unsigned short port(const char *s) {
  char *end = 0;
  const long sl = strtol(s, &end, 10);

  if (end == s || '\0' != *end ||
      ((LONG_MIN == sl || LONG_MAX == sl) && ERANGE == errno) || sl < 0 ||
      sl > USHRT_MAX) {
    log_print(LOG_ERROR, "port should in in the range of 1-65536: %s\n", s);
    return 1;
  }
  return (unsigned short)sl;
}

static void user(char *s, struct users *user) {
  char *p = strchr(s, ':');
  if (p == NULL) {
    log_print(LOG_ERROR, "password not found\n");
  } else {
    *p = 0;
    p++;
    if(strlen(s) > MAX_STR_SIZE || strlen(p) > MAX_STR_SIZE){
      log_print(LOG_ERROR, "username or password is too long");
      return;
    }
    strcpy(user->name, s);
    strcpy(user->pass, p);
    //user->name = s;
    //user->pass = p;
  }
}

static void version(void) {
  fprintf(stderr, "socks5v version 0.0\n"
                  "ITBA Protocolos de Comunicación 2022/1 -- Grupo 6\n");
}

static void usage(const char *progname) {
  fprintf(
      stderr,
      "Usage: %s [OPTION]...\n"
      "\n"
      "   -h               Imprime la ayuda y termina.\n"
      "   -l <SOCKS addr>  Dirección donde servirá el proxy SOCKS.\n"
      "   -L <conf  addr>  Dirección donde servirá el servicio de management.\n"
      "   -p <SOCKS port>  Puerto entrante conexiones SOCKS.\n"
      "   -P <conf port>   Puerto entrante conexiones configuracion\n"
      "   -u <name>:<pass> Usuario y contraseña de usuario que puede usar el "
      "proxy. Hasta 10.\n"
      "   -v               Imprime información sobre la versión versión y "
      "termina.\n"
      "   -N               Deshabilita los passwords disectors.\n"
      "\n",
      progname);
  exit(0);
}


int fill_ip_sockaddr(const char *ip, struct sockaddr_in* addrV4, struct sockaddr_in6* addrV6) {
  if (inet_pton(AF_INET, ip, &addrV4->sin_addr)) {
    return 4;
  } else if (inet_pton(AF_INET6, ip, &addrV6->sin6_addr)) {
    return 6;
  }
  return -1;
}

void fill_port_sockaddr(int port, struct sockaddr_in* addrV4, struct sockaddr_in6* addrV6) {
 addrV4->sin_port = htons(port);
 addrV6->sin6_port = htons(port);
}

void fill_sockaddr_v4(struct sockaddr_in* addr, int port, char* ip){
  memset(addr, 0, sizeof(*addr));
  
  addr->sin_family = AF_INET;
  inet_pton(AF_INET, ip, &addr->sin_addr);
  addr->sin_port = htons(port);
}

void fill_sockaddr_v6(struct sockaddr_in6* addr, int port, char* ip){
  memset(addr, 0, sizeof(*addr));

  addr->sin6_family = AF_INET6;
  inet_pton(AF_INET6, ip, &addr->sin6_addr);
  addr->sin6_port = htons(port);
}


