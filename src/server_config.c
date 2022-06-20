#include "../include/server_config.h"

int num_to_8bytes(buffer *buff_write , uint64_t n);
int num_to_4bytes(buffer *buff_write,uint32_t n);
int bool_to_bytes(buffer *buff_write, bool n);
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
int get_users(buffer *buff_write){
  size_t size;
  size_t n = 0;
  uint8_t *ptr = buffer_write_ptr(buff_write, &size);
  for (int i = 0; i < conf.users_size; i++) {
    char *user = conf.users[i].name;
    char *pass = conf.users[i].pass;
    for (int j = 0; user[j] != '\0' && size-3 > n; j++) {
      ptr[n] = user[j];
      n++;
    }
    ptr[n++] = ' ';
    for (int j = 0; user[j] != '\0' && size-2 > n; j++) {
      ptr[n] = pass[j];
      n++;
    }
    ptr[n++] = '\n';
  }
  ptr[n++] = '\0';

  buffer_write_adv(buff_write, n);
  return n;
}

int get_pop3_pass(buffer *buff_write){
  if(!sniffer_hast_next())
    begin_Sniffer_List();

  size_t size;
  size_t n = 0;
  uint8_t *ptr = buffer_write_ptr(buff_write, &size);

  while(sniffer_hast_next() && size > n){
    sniff_info *pop3_info = sniffer_get_next();
    char *user_socks = pop3_info->proxy_username;
    for (int j = 0; user_socks[j] != '\0' && size-3 > n; j++) {
      ptr[n] = user_socks[j];
      n++;
    }
    ptr[n++] = ' ';
    char *pop3_user = pop3_info->user;
    for (int j = 0; pop3_user[j] != '\0' && size-3 > n; j++) {
      ptr[n] = pop3_user[j];
      n++;
    }
    ptr[n++] = ' ';
    char *pop3_pass = pop3_info->passwd;
    for (int j = 0; pop3_pass[j] != '\0' && size-3 > n; j++) {
      ptr[n] = pop3_pass[j];
      n++;
    }
    ptr[n++] = ' ';
    char dest_ip[INET6_ADDRSTRLEN];
    ip_to_str(pop3_info->addr, dest_ip);
    for (int j = 0; dest_ip[j] != '\0' && size-2 > n; j++) {
      ptr[n] = dest_ip[j];
      n++;
    }
    ptr[n++] = '\n';
  }

  ptr[n++] = '\0';
  buffer_write_adv(buff_write, n);
  return n;
}

int get_buff_size(buffer *buff_write){
  uint32_t n = conf.socks_buffer_size;
  return num_to_4bytes(buff_write, n);
}

int get_auth_status(buffer *buff_write){
  bool n = is_auth_enabled();
  return bool_to_bytes(buff_write, n);
}

int get_spoofing_status(buffer *buff_write){
  bool n = is_spoofing_enabled();
  return bool_to_bytes(buff_write, n);
}

int _get_sent_bytes(buffer *buff_write){
  uint64_t n = get_sent_byte();
  return bool_to_bytes(buff_write, n);
}

int _get_received_bytes(buffer *buff_write){
  uint64_t n = get_sent_byte();
  return num_to_8bytes(buff_write, n);
}

int _get_transfered_bytes(buffer *buff_write){
  uint64_t n = get_sent_byte();
  return num_to_8bytes(buff_write, n);
}

int _get_current_conn(buffer *buff_write){
  uint32_t n = get_sent_byte();
  return num_to_4bytes(buff_write, n);
}

int _get_histori_conn(buffer *buff_write){
  uint32_t n = get_sent_byte();
  return num_to_4bytes(buff_write, n);
}

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
int bool_to_bytes(buffer *buff_write, bool n){
  size_t size;
  uint8_t *ptr = buffer_write_ptr(buff_write, &size);
  if(size < 1){
    return -1;
  }
  if(n){
    ptr[0] = 0x00;
  }
  ptr[0] = 0x01;

  buffer_write_adv(buff_write, 4);
  return 1;
}

int num_to_4bytes(buffer *buff_write,uint32_t n){
  size_t size;
  uint8_t *ptr = buffer_write_ptr(buff_write, &size);

  if(size < 4){
    return -1;
  }
  ptr[0] = (n >> 24) & 0xFF;
  ptr[1] = (n >> 16) & 0xFF;
  ptr[2] = (n >> 8) & 0xFF;
  ptr[3] = n & 0xFF;

  buffer_write_adv(buff_write, 4);
  return 4;
}


int num_to_8bytes(buffer *buff_write , uint64_t n){
  size_t size;
  uint8_t *ptr = buffer_write_ptr(buff_write, &size);

  if(size < 8){
    return -1;
  }
  for (int i = 0; i < 8; ++i) {
    ptr[i] = (n >> i*4 ) & 0xFF;
  }

  buffer_write_adv(buff_write, 8);
  return 8;
}

void ip_to_str(struct sockaddr_storage *addr, char *dest_ip){
  if(addr->ss_family == AF_INET){
    inet_ntop(AF_INET, &(((struct sockaddr_in *)addr)->sin_addr), dest_ip, INET_ADDRSTRLEN);
  }else{
    inet_ntop(AF_INET6, &(((struct sockaddr_in6 *)addr)->sin6_addr), dest_ip, INET6_ADDRSTRLEN);
  }
}

