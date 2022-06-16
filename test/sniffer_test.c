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
  char data[] = "+OK\nUSER fm\nPASS password\n+OK";
  log_print(INFO, "DATA SIZE %d.\n", N(data));
  FIXBUF(b, data);
  enum pop3_sniffer_state st = pop3_sniffer_consume(&b, &sniffer);
  log_print(INFO, "ST %d.\n", st);
  log_print(INFO, "LIST SIZE %d.\n", listSize(sniffer.list));
  listToBegin(sniffer.list);
	while(listHasNext(sniffer.list)){
		struct sniff_info sniffinfo = *((sniff_info*) listNext(sniffer.list));
    log_print(INFO, "USER %s.\n", sniffinfo.user);
    log_print(INFO, "PASSWD %s.\n", sniffinfo.passwd);
	}
}
