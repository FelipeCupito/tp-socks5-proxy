#include "../include/server_config.h"

int numTo8bytes(buffer *buff_write , uint64_t n);
int numTo4bytes(buffer *buff_write,uint32_t n);
int boolToBytes(buffer *buff_write, bool n);


config conf;




config* init_config(){
  conf.disectors_enabled = DEFAULT_DISECTORS;
  conf.mng_token = MNG_TOKEN;
  conf.socks_buffer_size = DEFAULT_SOCKS_BUFFER_SIZE;
  conf.users_size = 0;

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

int get_users(buffer *buff_write){
  size_t size;
  size_t n = 0;
  uint8_t *ptr = buffer_write_ptr(buff_write, &size);
  for (int i = 0; i < conf.users_size; i++) {
    char * user = conf.users[i].name;
    for (int j = 0; user[j] != '\0' && size-2 > n; j++) {
      ptr[n] = user[j];
      n++;
    }
    ptr[n++] = '\n';
  }
  ptr[n++] = '\0';

  buffer_write_adv(buff_write, n);
  return n;
}

int get_pass(buffer *buff_write){
  size_t size;
  size_t n = 0;
  uint8_t *ptr = buffer_write_ptr(buff_write, &size);
  for (int i = 0; i < conf.users_size; i++) {
    char * pass = conf.users[i].pass;
    for (int j = 0; pass[j] != '\0' && size-2 > n; j++) {
      ptr[n] = pass[j];
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
  return numTo4bytes(buff_write, n);
}

int get_auth_status(buffer *buff_write){
  bool n = is_auth_enabled();
  return boolToBytes(buff_write, n);
}

int get_spoofing_status(buffer *buff_write){
  bool n = is_spoofing_enabled();
  return boolToBytes(buff_write, n);
}

int _get_sent_bytes(buffer *buff_write){
  uint64_t n = get_sent_byte();
  return numTo8bytes(buff_write, n);
}

int _get_received_bytes(buffer *buff_write){
  uint64_t n = get_sent_byte();
  return numTo8bytes(buff_write, n);
}

int _get_transfered_bytes(buffer *buff_write){
  uint64_t n = get_sent_byte();
  return numTo8bytes(buff_write, n);
}

int _get_current_conn(buffer *buff_write){
  uint32_t n = get_sent_byte();
  return numTo4bytes(buff_write, n);
}

int _get_histori_conn(buffer *buff_write){
  uint32_t n = get_sent_byte();
  return numTo4bytes(buff_write, n);
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
int boolToBytes(buffer *buff_write, bool n){
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

int numTo4bytes(buffer *buff_write,uint32_t n){
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


int numTo8bytes(buffer *buff_write , uint64_t n){
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
