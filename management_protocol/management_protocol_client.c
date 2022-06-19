#include "../../include/management_protocol/management_protocol_client.h"

// funciones privadas
static void sigterm_handler(const int signal);

// variables
static bool done = false;

int main(const int argc, char** argv) {

    struct manage_args args;
    parse_args_handler(argc, argv, &args);

    args.authorized = false;

    int fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

    struct sockaddr_in socksaddr;
    socksaddr.sin_port = htons(args.mng_port);
    socksaddr.sin_family = AF_INET;
    inet_pton(AF_INET, args.mng_addr, &socksaddr.sin_addr);

    if (-1 == connect(fd, (struct sockaddr*) &socksaddr, sizeof(socksaddr))) {
        perror("couldn't connect");
        return -1;
    }

    signal(SIGTERM, sigterm_handler);
    signal(SIGINT, sigterm_handler);

    login(fd, &args);

    executeCommands(fd, &args);

    close(fd);
    return 0;
}

static void sigterm_handler(const int signal) {
  log_print(INFO, "signal %d, cleaning up and exiting ", signal);
  done = true;
}