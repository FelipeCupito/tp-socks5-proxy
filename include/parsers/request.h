#ifndef REQUEST_PARSER_H
#define REQUEST_PARSER_H

#include <netdb.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdint.h>
#include <errno.h>

#include "../buffer.h"
#include "../logger.h"
/*
+----+-----+-------+------+----------+----------+
|VER | CMD |  RSV  | ATYP | DST.ADDR | DST.PORT |
+----+-----+-------+------+----------+----------+
| 1  |  1  | X'00' |  1   | Variable |    2     |
+----+-----+-------+------+----------+----------+

esto se traduce en tener un estado para leer:

- version
- cmd
- rsv
- adddres
- dst addr
- dst port

*/

enum socks_req_cmd {
	socks_req_cmd_connect = 0x01,
	socks_req_cmd_bind = 0x02,
	socks_req_cmd_associate = 0x03,
};

enum socks_addr_type {
	socks_req_addrtype_ipv4 = 0x01,
	socks_req_addrtype_domain = 0x03,
	socks_req_addrtype_ipv6 = 0x04,
};

union socks_addr {
	char fqdn[0xff];
	struct sockaddr_in ipv4;
	struct sockaddr_in6 ipv6;
};

struct request {
	enum socks_req_cmd cmd;
	enum socks_addr_type dst_addr_type;
	union socks_addr dst_addr;
	in_port_t dst_port;
};

typedef enum request_state {
	request_version,
	request_cmd,
	request_rsv,
	request_atyp,
	request_dstaddr_fqdn,
	request_dstaddr,
	request_dstport,

	request_done,

	request_error,
	request_error_unsupported_version,
	request_error_unsupported_addresstype,
	request_error_unsupported_cmd
}request_state;

struct request_parser {
	struct request* request;
	enum request_state state;
	// cuantos bytes tenemos que leer
	uint8_t total_bytes_to_read;
	// cuantos ya leimos
	uint8_t bytes_read;
};

enum socks_response_status {
	status_succeeded = 0x00,
	status_general_SOCKS_server_failure = 0x01,
	status_connection_not_allowed_by_rules = 0x02,
	status_network_unreachable = 0x03,
	status_host_unreachable = 0x04,
	status_connection_refused = 0x05,
	status_ttl_expired = 0x06,
	status_command_not_supported = 0x07,
	status_address_type_not_supported = 0x08,
	status_close = 0x09,
};

void request_parser_init(struct request_parser* p);

enum request_state request_parser_feed(struct request_parser* p, const uint8_t c);

enum request_state request_consume(buffer* b, struct request_parser* p, bool* errored);

bool request_is_done(const enum request_state state, bool* errored);

void request_close(struct request_parser* p);

extern int request_marshall(buffer* b, const enum socks_response_status status, struct request* request);

enum socks_response_status errno_to_socks(int err);

enum socks_response_status cmd_resolve(struct request* request, struct sockaddr** originaddr, socklen_t* originlen, int* domain);

#endif