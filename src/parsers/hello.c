#include "../../include/parsers/hello.h"
#include "../../include/logger.h"
#include "../../include/buffer.h"

/*

enum hello_state {
  hello_version,
  // Estamos leyendo la cantidad de metodos
  hello_nmethods,
  // Estamos leyendo los metodos
  hello_methods,
  hello_done,
  hello_error_unsupported_version,
}

*/

/*extern*/ void hello_parser_init(struct hello_parser* p) {
	p->state = hello_version;
	p->remaining = 0;
}

extern enum hello_state hello_parser_feed(struct hello_parser* p, const uint8_t b) {
	switch (p->state) {
		case hello_version:
			if (b == 0x05) {
				p->state = hello_nmethods;
			}
			else {
				p->state = hello_error_unsupported_version;
			}
			break;

		case hello_nmethods:
			if (b <= 0) {
				p->state = hello_done;
			}
			p->remaining = b;
			p->state = hello_methods;
			break;

		case hello_methods:
			if (NULL != p->on_auth_method) {
				p->on_auth_method(p, b);
			}
			p->remaining--;
			if (p->remaining <= 0) {
				p->state = hello_done;
			}
			break;
		case hello_done:
			break;
		case hello_error_unsupported_version:
			break;
		default:
			log(FATAL, "Invalid state %d.\n", p->state);
	}

	return p->state;
}

extern bool hello_is_done(const enum hello_state state, bool* errored) {
	bool res;
	switch (state) {
		case hello_error_unsupported_version:
			if (errored != 0) {
				*errored = true;
			}
		case hello_done:
			res = true;
			break;
		default:
			res = false;
			break;
	}

	return res;
}

extern const char* hello_error(const struct hello_parser* p) {
	char* res;
	switch (p->state) {
		case hello_error_unsupported_version:
			res = "unsupported version";
			break;
		default:
			res = "";
			break;
	}

	return res;
}

extern void hello_parser_close(struct hello_parser* p) {
	//
}

extern enum hello_state hello_consume(buffer* buff, struct hello_parser* p, bool* errored) {
	enum hello_state state = p->state;

	while (buffer_can_read(buff)) {
		const uint8_t c = buffer_read(buff);
		state = hello_parser_feed(p, c);
		if (hello_is_done(state, errored)) {
			break;
		}
	}

	return state;
}

extern int hello_marshall(buffer* b, const uint8_t method) {
	size_t n;
	uint8_t* buff = buffer_write_ptr(b, &n);

	if (n < 2) {
		return -1;
	}

	buff[0] = 0x05;
	buff[1] = method;
	buffer_write_adv(b, 2);
	return 2;
}