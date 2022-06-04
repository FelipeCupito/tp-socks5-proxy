#include "../include/args.h"

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
    user->name = s;
    user->pass = p;
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

int get_sockaddr(const char *src, union IPAddress *sockaddr) {

  memset(sockaddr, 0, sizeof(sockaddr));

  if (inet_pton(AF_INET, src, &sockaddr->v4.sin_addr)) {
    sockaddr->v4.sin_family = AF_INET;
    return 4;
  } else if (inet_pton(AF_INET6, src, &sockaddr->v6.sin6_addr)) {
    sockaddr->v6.sin6_family = AF_INET6;
    return 6;
  }
  return -1;
}

void parse_args(const int argc, char **argv, serverConfig *config) {


  memset(config, 0, sizeof(serverConfig));

  // default config
  config->socks_sockaddr.v4.sin_family = AF_INET;
  inet_pton(AF_INET, DEFAULT_SOCKS_ADDRESS, &config->socks_sockaddr.v4.sin_addr);
  config->socks_sockaddr.v4.sin_port = htons(DEFAULT_SOCKS_PORT);
  config->is_socks_v4 = true;

  // mng config
  config->mng_sockaddr.v4.sin_family = AF_INET;
  inet_pton(AF_INET, DEFAULT_MNG_ADDRESS, &config->mng_sockaddr.v4.sin_addr);
  config->mng_sockaddr.v4.sin_port = htons(DEFAULT_MNG_PORT);
  config->is_mng_v4 = true;

  config->disectors_enabled = true;
  config->socks_buffer_size = DEFAULT_SOCKS_BUFFER_SIZE;
  config->timeout = SELECT_TIMEOUT;

  // guardo los argumentos
  int c;
  int nusers = 0;
  int flagIpMng = 0;
  int flagIpSocks = 0; 
  unsigned int portSocks = -1;
  unsigned int portMng = -1;

  while ((c = getopt(argc, argv, "hl:L:Np:P:u:v")) != -1) {

    switch (c) {
    case 'h':
      usage(argv[0]);
      break;
    case 'l':
      flagIpSocks = 1;
      int ipv = get_sockaddr(optarg, &config->socks_sockaddr);
      if (ipv == -1) {
        log_print(LOG_ERROR, "invalid server ip addr: %s\n", optarg);
        goto finally;
      }
      config->is_socks_v4 = (ipv == 4) ? true : false;
      break;
    case 'L':
      flagIpMng = 1;
      ipv = get_sockaddr(optarg, &config->mng_sockaddr);
      if (ipv == -1) {
        log_print(LOG_ERROR, "invalid Mng server ip addr: %s\n", optarg);
        goto finally;
      }
      config->is_mng_v4 = (ipv == 4) ? true : false;
      break;
    case 'N':
      config->disectors_enabled = false;
      break;
    case 'p':
      portSocks = port(optarg);
      break;
    case 'P':
      portMng = port(optarg);
      break;
    case 'u':
      if (nusers >= MAX_USERS) {
        log_print(LOG_ERROR, "maximum number of command line users reached: %d.\n",
            MAX_USERS);
      } else {
        user(optarg, config->users + nusers);
        nusers++;
      }
      break;
    case 'v':
      version();
      exit(0);
      break;
    default:
      log_print(LOG_ERROR, "unknown argument %d.\n", c);
    }
  }

  if(portSocks != -1){
    if(config->is_socks_v4)
      config->socks_sockaddr.v4.sin_port = htons(portSocks);
    else
      config->socks_sockaddr.v6.sin6_port = htons(portSocks);
  }

  if(flagIpMng && portMng != -1){
    if(config->is_mng_v4)
      config->mng_sockaddr.v4.sin_port = htons(portSocks);
    else
      config->mng_sockaddr.v6.sin6_port = htons(portSocks);
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
