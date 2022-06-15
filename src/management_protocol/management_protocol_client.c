#include <sys/socket.h>

#include "../../include/managment_protocol/args_handler.h"


int main(const int argc, char** argv) {

    struct mng_args args;
    parse_args(argc, argv, &args);

    int fd = socket(PF_INET, SOCK_STREAM, IPPROTO_SCTP);

    struct sockaddr_in sv;
    sv.sin_port = htons(args.mng_port);
    sv.sin_family = AF_INET;
    inet_pton(AF_INET, args.mng_addr, &sv.sin_addr);

    if (-1 == connect(fd, (struct sockaddr*) &sv, sizeof(sv))) {
        perror("couldn't connect");
        return -1;
    }

    // signal(SIGTERM, sigterm_handler);
    // signal(SIGINT, sigterm_handler);

    input_init();

    login(fd, &args.user);

    menu(fd);

    close(fd);
    status = success;
    return 0;
}