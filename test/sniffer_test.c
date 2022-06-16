#include "../include/pop3_sniffer.h"
#include "../include/buffer.h"

#define N(x) (sizeof(x) / sizeof(x[0]))

#define FIXBUF(b, data)                 \
    buffer_init(&(b), N(data), (data)); \
    buffer_write_adv(&(b), N(data))

int main(int argc, char const *argv[]) {
  struct pop3_sniffer sniffer;
  pop3_sniffer_init(&sniffer);
  buffer b;
  char* data = "+OK\nUSER fm\nPASS password\n+OK";
  FIXBUF(b, data);
  enum pop3_sniffer_state st = pop3_sniffer_consume(&b, &sniffer);
  return 0;
}
