#include "../include/pop3_sniffer.h"
#define SIZEOF(arr) strlen(arr) * sizeof(char *)

void* freeElem(void * elem) {
  sniff_info *sniffinfo = (sniff_info*) elem;
  free(sniffinfo -> user);
  free(sniffinfo -> passwd);
	free(sniffinfo);
  return 0;
}

void freeSniffer(struct pop3_sniffer* s) {
  freeList(s -> list);
}

static void reset_read(struct pop3_sniffer* s, uint8_t remain){
  s -> read = 0;
  s -> remaining = remain;
}

void pop3_sniffer_init_list(struct pop3_sniffer* s){
  s -> list = newList(freeElem);
}
void pop3_sniffer_init(struct pop3_sniffer* s){
  s -> state = pop3_sniffer_initial;
  reset_read(s,strlen(OK));
}

static enum pop3_sniffer_state ok(struct pop3_sniffer* s,uint8_t b){
  char a = toupper(b);
  char x = toupper(*(OK + s -> read));
  if(toupper(b) == toupper(*(OK + s -> read))) {
    s -> read++;
    s -> remaining--;
    if(s -> remaining == 0) {
      s -> state = pop3_sniffer_user;
      reset_read(s,strlen(USER));
    }
  }
  else {
      s -> state = pop3_sniffer_err;
  }
  return s -> state;
}

enum pop3_sniffer_state user(struct pop3_sniffer* s,uint8_t b){
  char a = toupper(b);
  if(toupper(b) == toupper(*(USER + s -> read))) {
    s -> read++;
    s -> remaining--;
    if(s -> remaining == 0) {
      s -> read = 0;
      s -> state = pop3_sniffer_read_user;
    }        
  }
  else {
    if(s -> read != 0) {
      reset_read(s,strlen(USER));
    }
  }

  return s -> state;
}

enum pop3_sniffer_state read_user(struct pop3_sniffer* s,uint8_t b){
  if(b != '\n') {
    if(s -> read < MAX_USR_PASS_SIZE){
      s -> username[s -> read++] = b;
    }
  }
  else {
    s -> username[s -> read] = '\0';
    s -> read = 0;
    s -> remaining = strlen(PASS);
    s -> check_read = 0;
    s -> check_remaining = strlen(ERR);
    s -> state = pop3_sniffer_password;
  }

  return s -> state;
}

enum pop3_sniffer_state password(struct pop3_sniffer* s,uint8_t b){
  if(toupper(b) == toupper(*(PASS + s -> read))) {
    s -> read++;
    s -> remaining--;
    if(s -> remaining == 0) {
      s -> read = 0;
      s -> state = pop3_sniffer_read_passsword;
    }        
  }
  else {
    if(s -> read != 0) {
      reset_read(s,strlen(PASS));
    }
  }

  return s -> state;
}

enum pop3_sniffer_state read_password(struct pop3_sniffer* s,uint8_t b) {
  if(b != '\n'){
    if(s -> read < MAX_USR_PASS_SIZE){
      s -> passwd[s -> read++] = b;
    }
  }
  else{
    s -> passwd[s -> read] = '\0';
    s -> read = 0;
    s -> check_read = 0;
    s -> state = pop3_sniffer_check_password; 
  }
  return s -> state;
}

enum pop3_sniffer_state check_password(struct pop3_sniffer* s,uint8_t b){
  if(toupper(b) == toupper(*(OK + s -> read))){
    s -> read++;
    if(s -> read == strlen(OK)){
      s -> state = pop3_sniffer_ok;
    }
  }
  else if(toupper(b) == toupper(*(ERR + s -> check_read))){
    s -> check_read++;
    if(s -> check_read == strlen(ERR)){
      s -> state = pop3_sniffer_user;
    }
  }
  return s -> state;
}

enum pop3_sniffer_state pop3_sniffer_parse(struct pop3_sniffer* s,uint8_t b){
  switch (s -> state) {
    case pop3_sniffer_initial:
      s -> state = ok(s,b);
      break;
    case pop3_sniffer_user:
      s -> state = user(s,b);
      break;
    case pop3_sniffer_read_user:
      s -> state = read_user(s,b);
      break;
    case pop3_sniffer_password:
      s -> state = password(s,b);
      break;
    case pop3_sniffer_read_passsword:
      s -> state = read_password(s,b);
      break;
    case pop3_sniffer_check_password:
      s -> state = check_password(s,b);
      break;    
    case pop3_sniffer_ok:
    case pop3_sniffer_err:
      break;    
    default:
      break;
  }
  return s -> state;
}

bool pop3_is_done(struct pop3_sniffer *s) {
  return s -> state == pop3_sniffer_ok || s -> state == pop3_sniffer_err;
}

bool pop3_is_parsing(struct pop3_sniffer *s){
    return s -> state >= pop3_sniffer_initial && s -> state < pop3_sniffer_ok;
}

enum pop3_sniffer_state pop3_sniffer_consume(buffer *buff, struct pop3_sniffer *s){
  while(buffer_can_read(buff) && !pop3_is_done(s)) {
    uint8_t b = buffer_read(buff);
    pop3_sniffer_parse(s,b);
  }

  if(s -> state == pop3_sniffer_ok){
    struct sniff_info *sniffinfo = malloc (sizeof (struct sniff_info));
    if (sniffinfo == NULL){
      return pop3_sniffer_err;
    }

    sniffinfo -> user = malloc(SIZEOF(s -> username));
    if (sniffinfo -> user == NULL) {
      return pop3_sniffer_err;
    }
    memcpy(sniffinfo -> user, s -> username, SIZEOF(s -> username));

    sniffinfo -> passwd = malloc(SIZEOF(s -> passwd));
    if (sniffinfo -> passwd == NULL) {
      return pop3_sniffer_err;
    }
    memcpy(sniffinfo -> passwd, s -> passwd, SIZEOF(s -> passwd));

    insert(s -> list, sniffinfo);
  }
  return s -> state;
}

// static void pop3sniff(struct pop3_sniffer *s, uint8_t *ptr, ssize_t size){
//     if(!pop3_is_parsing(s)){
//         pop3_sniffer_init(s);
//     }
//     if(!pop3_is_done(s)){
//         size_t count;
//         uint8_t *pop3_ptr = buffer_write_ptr(&s -> buffer,&count);
//         // Pierdo info :/
//         if((unsigned) size <= count){
//             memcpy(pop3_ptr,ptr,size);
//             buffer_write_adv(&s -> buffer,size);
//         }
//         else{
//             memcpy(pop3_ptr,ptr,count);
//             buffer_write_adv(&s -> buffer,count);
//         }
//         pop3_consume(s,&ATTACHMENT(key) -> socks_info);
//     }
// }