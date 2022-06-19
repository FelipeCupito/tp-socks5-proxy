#include "../include/server_config.h"


config conf;


config* init_config(){
  conf.disectors_enabled = DEFAULT_DISECTORS;
  conf.socks_buffer_size = DEFAULT_SOCKS_BUFFER_SIZE;
  conf.users_size = 0;
  return &conf;
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

int is_auth_enabled(){
    return conf.users_size > 0;
}