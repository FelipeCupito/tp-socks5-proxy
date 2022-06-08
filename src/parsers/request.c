#include "../../include/parsers/request.h"

static void remaining_set(struct request_parser* p, const int n) {
	p->bytes_read = 0;
	p->total_bytes_to_read = n;
}

static int remaining_is_done(struct request_parser* p) {
	return p->bytes_read >= p->total_bytes_to_read;
}

static enum request_state version(const uint8_t c, struct request_parser* p) {
	enum request_state next;

	if (c == 0x05) {
		next = request_cmd;
	}
	else {
		next = request_error_unsupported_version;
	}

	return next;
}

static enum request_state cmd(const uint8_t c, struct request_parser* p) {
	p->request->cmd = c;
	return request_rsv;
}

static enum request_state rsv(const uint8_t c, struct request_parser* p) {
	return request_atyp;
}

static enum request_state atyp(const uint8_t c, struct request_parser* p) {
	enum request_state next;
	p->request->dst_addr_type = c;

	switch (p->request->dst_addr_type) {
		case socks_req_addrtype_ipv4:
			remaining_set(p, 4);
			memset(&(p->request->dst_addr.ipv4), 0, sizeof(p->request->dst_addr.ipv4));
			p->request->dst_addr.ipv4.sin_family = AF_INET;
			next = request_dstaddr;
			break;

		case socks_req_addrtype_ipv6:
			remaining_set(p, 16);
			memset(&(p->request->dst_addr.ipv6), 0, sizeof(p->request->dst_addr.ipv6));
			p->request->dst_addr.ipv6.sin6_family = AF_INET6;
			next = request_dstaddr;
			break;

		case socks_req_addrtype_domain:
			next = request_dstaddr_fqdn;
			break;

		default:
			next = request_error_unsupported_addresstype;
			break;
	}

	return next;
}

static enum request_state dstaddr_fqdn(const uint8_t c, struct request_parser* p) {
	// el byte nos dice cuanto nos queda por leer
	remaining_set(p, c);
	// TODO este puede llegar a ser bytes_read
	p->request->dst_addr.fqdn[p->total_bytes_to_read - 1] = 0;

	return request_dstaddr;
}

static enum request_state dstaddr(const uint8_t c, struct request_parser* p) {
	enum request_state next;

	// TODO no entiendo esto
	// TODO completar
	switch (p->request->dst_addr_type) {
		case socks_req_addrtype_ipv4:
			// TODO completar
			// Esta linea se ve cortada en la clase
			((uint8_t*) &(p->request->dst_addr.ipv4.sin_addr))[p->bytes_read++] = c;
			break;
		case socks_req_addrtype_ipv6:
			((uint8_t*) &(p->request->dst_addr.ipv6.sin6_addr))[p->bytes_read++] = c;
			break;
		case socks_req_addrtype_domain:
			p->request->dst_addr.fqdn[p->bytes_read++] = c;
			break;
	}

	if (remaining_is_done(p)) {
		remaining_set(p, 2);
		p->request->dst_port = 0;
		next = request_dstport;
	}
	else {
		next = request_dstaddr;
	}

	return next;
}

static enum request_state dstport(const uint8_t c, struct request_parser* p) {
	enum request_state next;
	// TODO no entiendo que es esto
	*(((uint8_t*) &(p->request->dst_port)) + p->bytes_read) = c;
	p->bytes_read++;
	if (p->bytes_read >= p->total_bytes_to_read) {
		next = request_done;
	}
	else {
		next = request_dstport;
	}

	return next;
}

void request_parser_init(struct request_parser* p) {
	p->state = request_version;
	memset(p->request, 0, sizeof(*(p->request)));
}

extern enum request_state request_parser_feed(struct request_parser* p, const uint8_t c) {
	enum request_state next;

	switch (p->state) {
		case request_version:
			next = version(c, p);
			break;
		case request_cmd:
			next = cmd(c, p);
			break;
		case request_rsv:
			next = rsv(c, p);
			break;
		case request_atyp:
			next = atyp(c, p);
			break;
		case request_dstaddr_fqdn:
			next = dstaddr_fqdn(c, p);
			break;
		case request_dstaddr:
			next = dstaddr(c, p);
			break;
		case request_dstport:
			next = dstport(c, p);
			break;
		case request_done:
		case request_error:
		case request_error_unsupported_version:
		case request_error_unsupported_addresstype:
		case request_error_unsupported_cmd:
			next = p->state;
			break;
		default:
			next = request_error;
			break;
	}
	p->state = next;
	return next;
}

enum request_state request_consume(buffer* b, struct request_parser* p, bool* error) {
	enum request_state st = p->state;
	while (buffer_can_read(b)) {
		uint8_t byte = buffer_read(b);
		st = request_parser_feed(p, byte);
		if (request_is_done(st, error)) {
			break;
		}
	}
	return st;
}

extern bool request_is_done(const enum request_state state, bool* errored) {
	bool ret = false;
	switch (request_state) {
		case request_error:
		case request_error_unsupported_version:
		case request_error_unsupported_addresstype:
		case request_error_unsupported_cmd:
			if (errored) {
				*errored = true;
			}
			ret = true;
			break;
		case request_done:
			ret = true;
			break;
		default:
			ret = false;
			break;
	}
	return ret;
}

extern int request_marshall(buffer* b, const enum socks_response_status status, struct request* request) {
	const enum socks_atyp atyp = request->dst_addr_type;
	const union socks_addr addr = request->dst_addr;
	const in_port_t dest_port = request->dst_port;
	size_t n, len = 6;
	uint8_t* buff = buffer_write_ptr(b, &n);
	uint8_t* aux = NULL;
	int address_size = 0;

	switch (atyp) {
		case socks_req_addrtype_ipv4:
			address_size = 4;
			len = len + address_size;
			aux = (uint8_t*) malloc(4 * sizeof(uint8_t));
			memcpy(aux, &addr.ipv4.sin_addr, 4);
			break;
		case socks_req_addrtype_ipv6:
			address_size = 16;
			len = len + address_size;
			aux = (uint8_t*) malloc(16 * sizeof(uint8_t));
			memcpy(aux, &addr.ipv6.sin6_addr, 16);
			break;
		case socks_req_addrtype_domain:
			address_size = strlen(addr.fqdn);
			aux = (uint8_t*) malloc((address_size + 1) * sizeof(uint8_t));
			aux[0] = address_size;
			memcpy(aux + 1, addr.fqdn, address_size);
			address_size++;
			len = len + address_size;
			break;
	}

	if (n < len) {
		refree(aux);
		return -1;
	}

	buff[0] = 0x05;
	buff[1] = status;
	buff[2] = 0x00;
	buff[3] = atyp;
	memcpy(&buff[4], aux, address_size);
	free(aux);
	memcpy(&buff[4 + addaddress_size & dest_port, 2);
	buffer_write_adv(b, len);
	return len;
}

enum socks_response_status errno_to_socks(int err) {
	enum socks_reply reply;

	switch (err) {
		case 0:
			reply = status_succeeded;
			break;
		case EHOSTUNREACH :
			reply = status_host_unreachable;
			break;
		case ECONNREFUSED :
			reply = status_connection_refused;
			break;
		case ETIMEDOUT:
			reply = status_ttl_expired;
			break;
		case EAFNOSUPPORT:
			reply = status_address_type_not_supported;
			break;
		case ENETUNREACH:
			reply = status_network_unreachable;
			break;
		default:
			reply = status_general_SOCKS_server_failure;
	}

	return reply;
}

enum socks_response_status cmd_resolve(
	struct request* request, 
	struct sockaddr** originaddr, 
	socklen_t* originlen, 
	int* domain) {
		enum socks_reply_status status = status_succeeded;
		//*domain = AF_INET;
		struct sockaddr *address;
		socklen_t len = 0;

		if (request -> dst_addr_type == socks_req_addrtype_domain) {
			struct hostent *host = gethostbyname(request -> dst_addr.fqdn);
			if (host == 0) {
				memset(&request -> dst_addr, 0x00, sizeof(request -> dst_addr));
				return status_host_unreachable;
			} 
			else {
				request -> dst_addr.ipv4.sin_family = host -> h_addrtype;
				memcpy((char *)&request->dst_addr.ipv4.sin_addr, *hp->h_addr_list, hp->h_length);
			}
		}

		if (request -> dst_addr_type == socks_req_addrtype_ipv4) {
			*domain = AF_INET;
			address = (struct sockaddr *)&(request -> dst_addr.ipv4);
			len = sizeof(request -> dst_addr.ipv4);
			request -> dst_addr.ipv4.sin_port = request -> dst_port;
		} 
		else if (request -> dst_addr_type == socks_req_addrtype_ipv6) {
			*domain = AF_INET6;
			address = (struct sockaddr *)&(request->dst_addr.ipv6);
			len = sizeof(request->dst_addr.ipv6);
			request -> dst_addr.ipv6.sin6_port = request->dst_port;
		} else {
			return status_address_type_not_supported;
		}

		*originaddr = address;
    *originlen = len;

    return status;
}