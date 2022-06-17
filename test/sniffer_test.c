#include "../include/pop3_sniffer.h"
#include "../include/buffer.h"

#define N(x) (sizeof(x) / sizeof(x[0]))

#define FIXBUF(b, data)                 \
    buffer_init(&(b), N(data), (data)); \
    buffer_write_adv(&(b), N(data))

int main(int argc, char const *argv[]) {
  struct pop3_sniffer sniffer;
  pop3_sniffer_init(&sniffer);
  pop3_sniffer_init_list(&sniffer);

  buffer b;
  char data[] = "+OK\nUSER fm\nPASS password\n+OK";
  FIXBUF(b, data);
  log_print(INFO, "SIZE %d.\n", N(data));
  enum pop3_sniffer_state st = pop3_sniffer_consume(&b, &sniffer);
  log_print(INFO, "ST %d.\n", st);
  log_print(INFO, "LIST SIZE %d.\n", listSize(sniffer.list));

  buffer d;
  pop3_sniffer_init(&sniffer);
  char third_data[] = "+O\nUSERPACXO\nPASS JUANPEREX\n-ERR";
  FIXBUF(d, third_data);
  st = pop3_sniffer_consume(&d, &sniffer);
  log_print(INFO, "SIZE %d.\n", N(third_data));
  log_print(INFO, "ST %d.\n", st);
  log_print(INFO, "LIST SIZE %d.\n", listSize(sniffer.list));

  buffer c;
  pop3_sniffer_init(&sniffer);
  char second_data[] = "+OK\nUSER fmaggioni\nPASS passworddddd\n+OK";
  FIXBUF(c, second_data);
  st = pop3_sniffer_consume(&c, &sniffer);
  log_print(INFO, "SIZE %d.\n", N(second_data));
  log_print(INFO, "ST %d.\n", st);
  log_print(INFO, "LIST SIZE %d.\n", listSize(sniffer.list));

  buffer x;
  pop3_sniffer_init(&sniffer);
  char four_data[] = "+OK\nUSER Ginobili\nPASS SCOLa\n+OK";
  FIXBUF(x, four_data);
  st = pop3_sniffer_consume(&x, &sniffer);  
  log_print(INFO, "SIZE %d.\n", N(four_data));
  log_print(INFO, "ST %d.\n", st);
  log_print(INFO, "LIST SIZE %d.\n", listSize(sniffer.list));

  listToBegin(sniffer.list);
	while(listHasNext(sniffer.list)){
		struct sniff_info* sniffinfo = ((sniff_info*) listNext(sniffer.list));
    log_print(INFO, "USER %s.\n", sniffinfo -> user);
    log_print(INFO, "PASSWD %s.\n", sniffinfo -> passwd);
	}

  freeSniffer(&sniffer);
}
