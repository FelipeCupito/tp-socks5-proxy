#include "../../include/parsers/request.h";

static void remaining_set (struct request_parser* p, const int n) {
  p -> bytes_read = 0;
  p -> total_bytes_to_read = n;
}

static int remaining_is_done (struct request_parser* p) {
  return p->bytes_read >= p -> total_bytes_to_read;
}

static enum request_state version (const uint8_t c, struct request_parser* p) {
  enum request_state next;

  if (c == 0x05) {
    next = request_cmd;
  } else {
    next = reques_error_unsupported_version;
  }

  return next;
}

static enum request_state cmd (const uint8_t c, struct request_parser* p) {
  p -> request -> cmd = c;
  return request_rsv;
}

static enum request_state rsv (const uint8_t c, struct request_parser* p) {
  return request_atyp;
}

static enum request_state atyp (const uint8_t c, struct request_parser* p) {
  enum request_state next;
  p -> request -> dst_addr_type = c;

  switch (p -> request -> dst_addr_type) {
    case socks_req_addrtype_ipv4:
      remaining_set(p,4);
      memset(&(p -> request -> dst_addr.ipv4), 0, sizeof(p -> request -> dst_addr.ipv4));
      p -> request -> dst_addr.ipv4.sin_family = AF_INET;
      next = request_dstaddr;
      break;
    
    case socks_req_addrtype_ipv6:
      remaining_set(p,16);
      memset(&(p -> request -> dst_addr.ipv6), 0, sizeof(p -> request -> dst_addr.ipv6));
      p -> request -> dst_addr.ipv6.sin_family = AF_INET6;
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

static enum request_state dst_addr_fqdn(const uint8_t c, struct request_parser* p) {
  remaining_set(p,c);
  // TODO este puede llegar a ser bytes_read
  p -> request -> dst_addr.fqdn[p->total_bytes_to_read - 1] = 0; 

  return request_dstaddr;
}

static enum request_state dstaddr(const uint8_t c, struct request_parser* p) {
  enum request_state next;

  switch (p -> request -> dst_addr_type) {
    case socks_req_addrtype_ipv4:
      ((uint8_t *)&(p -> request -> dst_addr.ipv4.sin_addr))[]
      break;
  }
}