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
	// TODO no entendi que hace
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
			next = p->state;
			break;
		default:
			next = request_error;
			break;
	}
	p -> state = next
	return next;
}

enum request_state request_consume(buffer *b, request_parser *p, bool *error)
{
    enum request_state st = p->state;
    while (buffer_can_read(b) && !finished)
    {
        uint8_t byte = buffer_read(b);
        st = request_parser_feed(p, byte);
        if (request_is_done(st, error))
        {
            break;
        }
    }
    return st;
}

int request_marshall( buffer *b, 
										 const enum socks_reply_status status, 
										 const enum socks_atyp atyp, 
										 const union socks_addr addr, 
										 const in_port_t dest_port ) 
{
    size_t n, len = 6;
    uint8_t *buff = buffer_write_ptr(b, &n);
    uint8_t *aux = NULL;
    int addr_size = 0;
    switch (atyp) {
    case ipv4_type:
        addr_size = 4;
        len += addr_size;
        aux = (uint8_t *)malloc(4 * sizeof(uint8_t));
        memcpy(aux, &addr.ipv4.sin_addr, 4);
        break;
    case ipv6_type:
        addr_size = 16;
        len += addr_size;
        aux = (uint8_t *)malloc(16 * sizeof(uint8_t));
        memcpy(aux, &addr.ipv6.sin6_addr, 16);
        break;
    case domainname_type:
        addr_size = strlen(addr.fqdn);
        aux = (uint8_t *)malloc((addr_size + 1) * sizeof(uint8_t));
        aux[0] = addr_size;
        memcpy(aux + 1, addr.fqdn, addr_size);
        addr_size++;
        len += addr_size;
        break;
    }
    if (n < len) {
        free(aux);
        return -1;
    }
    buff[0] = 0x05;
    buff[1] = status;
    buff[2] = 0x00;
    buff[3] = atyp;
    memcpy(&buff[4], aux, addr_size);
    free(aux);
    memcpy(&buff[4 + addr_size], &dest_port, 2);
    buffer_write_adv(b, len);
    return len;
}