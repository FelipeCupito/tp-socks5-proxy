#include "./include/management_protocol_client.h"

// funciones privadas
static void sigterm_handler(const int signal);
static int connect_ipv4(struct manage_args* args);
static int connect_ipv6(struct manage_args* args);

// variables
static bool done = false;

int main(const int argc, char** argv) {

	int fd = -1;
	struct manage_args args;
	parse_args_handler(argc, argv, &args);

	args.authorized = false;

	struct in_addr addrv4;
	struct in6_addr addrv6;

	if (inet_pton(AF_INET, args.mng_addr, &addrv4)) {
		fd = connect_ipv4(&args);
	}
	else if (inet_pton(AF_INET6, args.mng_addr, &addrv6)) {
		fd = connect_ipv6(&args);
	}
	else {
		log_print(FATAL, "Invalid IP address provided.");
	}
	if (fd == -1)
		log_print(FATAL, "Connection failed.");

	signal(SIGTERM, sigterm_handler);
	signal(SIGINT, sigterm_handler);

	if (args.try_password == NULL) {
		close(fd);
		log_print(FATAL, "Authentication required");
	}

	login(fd, &args);

	if (!args.authorized) {
		close(fd);
		log_print(FATAL, "Authentication failed");
	}

	execute_commands(fd, &args);

	close(fd);
	return 0;
}

static void sigterm_handler(const int signal) {
	printf("signal %d, cleaning up and exiting\n", signal);
	done = true;
}

static int connect_ipv4(struct manage_args* args) {

	int fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (fd == -1) {
		return -1;
	}

	struct sockaddr_in socksaddr;

	socksaddr.sin_port = htons(args->mng_port);
	socksaddr.sin_family = AF_INET;

	inet_pton(AF_INET, args->mng_addr, &socksaddr.sin_addr);

	if (-1 == connect(fd, (struct sockaddr*) &socksaddr, sizeof(socksaddr))) {
		return -1;
	}

	return fd;
}


static int connect_ipv6(struct manage_args* args) {

	int fd = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);

	if (fd == -1) {
		return -1;
	}

	struct sockaddr_in6 socksaddr6;

	socksaddr6.sin6_port = htons(args->mng_port);
	socksaddr6.sin6_family = AF_INET6;

	inet_pton(AF_INET6, args->mng_addr, &socksaddr6.sin6_addr);

	if (-1 == connect(fd, (struct sockaddr*) &socksaddr6, sizeof(socksaddr6))) {
		return -1;
	}

	return fd;
}