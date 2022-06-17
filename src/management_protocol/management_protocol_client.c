#include <sys/socket.h>
#include <sys/types.h> 
#include <signal.h>

#include "../../include/management_protocol/args_handler.h"

// funciones privadas
static void sigterm_handler(const int signal);

// variables
static bool done = false;

int main(const int argc, char** argv) {

    struct manage_args args;
    parse_args(argc, argv, &args);

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

    login(fd, args.try_password);

    close(fd);
    return 0;
}

static void sigterm_handler(const int signal) {
  log_print(INFO, "signal %d, cleaning up and exiting ", signal);
  done = true;
}