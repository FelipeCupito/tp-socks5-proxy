#include "../include/server_config.h"

int num_to_8bytes(char *res, int res_size, uint32_t n);
int num_to_4bytes(char *res, int res_size, uint32_t n);
int bool_to_bytes(char *res, int res_size, bool n);
void ip_to_str(struct sockaddr_storage *addr, char *dest_ip);

config conf;




config* init_config(){
  conf.disectors_enabled = DEFAULT_DISECTORS;
  conf.mng_token = MNG_TOKEN;
  conf.socks_buffer_size = DEFAULT_SOCKS_BUFFER_SIZE;
  conf.users_size = 0;

  pop3_sniffer_init_list();

  return &conf;
}

int checkToken(char *token){
  if(strcmp(conf.mng_token, token) == 0){
    return 1;
  }
  return 0;
}

int checkUser(char *user, char *pass){
    for (int i = 0; i < conf.users_size; i++){
      struct users *current_user =  &conf.users[i];
      if(strcmp(current_user->name, user)== 0){
            if(strcmp(current_user->pass, pass)== 0){
                return 1;
            }
        }
    }
    return 0;
}
////////////////////////////////////////////////////////
//                  GETTERS
////////////////////////////////////////////////////////
int get_users(char *res, int res_size){
  //size_t size;
  size_t n = 0;
  //uint8_t *ptr = buffer_write_ptr(buff_write, &size);
  for (int i = 0; i < conf.users_size; i++) {
    char *user = conf.users[i].name;
    char *pass = conf.users[i].pass;
    for (int j = 0; user[j] != '\0' && res_size-3 > n; j++) {
      res[n] = user[j];
      n++;
    }
    res[n++] = ' ';
    for (int j = 0; user[j] != '\0' && res_size-2 > n; j++) {
      res[n] = pass[j];
      n++;
    }
    res[n++] = '\n';
  }
  res[n++] = '\0';

  //buffer_write_adv(buff_write, n);
  return n;
}

int get_pop3_pass(char *res, int res_size){
  if(!sniffer_hast_next())
    begin_Sniffer_List();

  //size_t size;
  size_t n = 0;
  //uint8_t *ptr = buffer_write_ptr(buff_write, &size);

  while(sniffer_hast_next() && res_size > n){
    sniff_info *pop3_info = sniffer_get_next();
    char *user_socks = pop3_info->proxy_username;
    for (int j = 0; user_socks[j] != '\0' && res_size-3 > n; j++) {
      res[n] = user_socks[j];
      n++;
    }
    res[n++] = ' ';
    char *pop3_user = pop3_info->user;
    for (int j = 0; pop3_user[j] != '\0' && res_size-3 > n; j++) {
      res[n] = pop3_user[j];
      n++;
    }
    res[n++] = ' ';
    char *pop3_pass = pop3_info->passwd;
    for (int j = 0; pop3_pass[j] != '\0' && res_size-3 > n; j++) {
      res[n] = pop3_pass[j];
      n++;
    }
    res[n++] = ' ';
    char dest_ip[INET6_ADDRSTRLEN];
    ip_to_str(pop3_info->addr, dest_ip);
    for (int j = 0; dest_ip[j] != '\0' && res_size-2 > n; j++) {
      res[n] = dest_ip[j];
      n++;
    }
    res[n++] = '\n';
  }

  res[n++] = '\0';
  //buffer_write_adv(buff_write, n);
  return n;
}

int get_buff_size(char *res, int res_size){
  uint32_t n = conf.socks_buffer_size;
  return num_to_4bytes(res, res_size, n);
}

int get_auth_status(char *res, int res_size){
  bool n = is_auth_enabled();
  return bool_to_bytes(res, res_size, n);
}

int get_spoofing_status(char *res, int res_size){
  bool n = is_spoofing_enabled();
  return bool_to_bytes(res, res_size, n);
}

int _get_sent_bytes(char *res, int res_size){
  uint64_t n = get_sent_byte();
  return bool_to_bytes(res, res_size, n);
}

int _get_received_bytes(char *res, int res_size){
  uint64_t n = get_sent_byte();
  return num_to_8bytes(res, res_size, n);
}

int _get_transfered_bytes(char *res, int res_size){
  uint64_t n = get_sent_byte();
  return num_to_8bytes(res, res_size, n);
}

int _get_current_conn(char *res, int res_size){
  uint32_t n = get_sent_byte();
  return num_to_4bytes(res, res_size, n);
}

int _get_histori_conn(char *res, int res_size){
  uint32_t n = get_sent_byte();
  return num_to_4bytes(res, res_size, n);
}


////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////
int is_spoofing_enabled(){
  return conf.disectors_enabled;
}

int is_auth_enabled(){
    return conf.auth_enabled;
}



////////////////////////////////////////
//privadas
///////////////////////////////////////////

//o ON X'00'
//o OFF X'01'
int bool_to_bytes(char *res, int res_size, bool n){
  //size_t size;
 // uint8_t *ptr = buffer_write_ptr(buff_write, &size);
  if(res_size < 1){
    return -1;
  }
  if(n){
    res[0] = 0x00;
  }
  res[0] = 0x01;

  //buffer_write_adv(buff_write, 4);
  return 1;
}

int num_to_4bytes(char *res, int res_size, uint32_t n){
  //size_t size;
  //uint8_t *ptr = buffer_write_ptr(buff_write, &size);

  if(res_size < 4){
    return -1;
  }
  res[0] = (n >> 24) & 0xFF;
  res[1] = (n >> 16) & 0xFF;
  res[2] = (n >> 8) & 0xFF;
  res[3] = n & 0xFF;

  //buffer_write_adv(buff_write, 4);
  return 4;
}


int num_to_8bytes(char *res, int res_size, uint32_t n){
  //size_t size;
  //uint8_t *ptr = buffer_write_ptr(buff_write, &size);

  if(res_size < 8){
    return -1;
  }
  for (int i = 0; i < 8; ++i) {
    res[i] = (n >> i*4 ) & 0xFF;
  }

  //buffer_write_adv(buff_write, 8);
  return 8;
}

void ip_to_str(struct sockaddr_storage *addr, char *dest_ip){
  if(addr->ss_family == AF_INET){
    inet_ntop(AF_INET, &(((struct sockaddr_in *)addr)->sin_addr), dest_ip, INET_ADDRSTRLEN);
  }else{
    inet_ntop(AF_INET6, &(((struct sockaddr_in6 *)addr)->sin6_addr), dest_ip, INET6_ADDRSTRLEN);
  }
}

