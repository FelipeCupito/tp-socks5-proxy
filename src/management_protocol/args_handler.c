#include "../../include/management_protocol/args_handler.h"

struct manage_args {
    char* manage_addr_v4;
    // char* manage_addr_v6;
    // bool isIpV6;
    unsigned int port;
    uint8_t version;

    char* try_password;
} manage_args;

// Private functions
static void mng_usage();
static unsigned short port(const char* s);

void parse_args(const int argc, char** argv, manage_args* mng_args) {
    memset(mng_args, 0, sizeof(struct manage_args));

    // Set default socks for our protocol
    mng_args->manage_addr_v4 = DEFAULT_MNG_ADDR_V4;
    mng_args->port = DEFAULT_MNG_PORT;
    int c;

    while (c = getopt(argc, argv, "hL:P:a:v") != -1) {
        switch (c) {
            case 'h':
                // Help
                mng_usage();
                break;
            case 'L':
                mng_args->manage_addr_v4 = optarg;
                break;
            case 'P':
                mng_args->port = port(optarg);
                break;
            case 'a':
                mng_args->password = optarg;
                break;
            case 'v':
                // mensaje de la version
                mng_args->version = atoi(optarg);
                exit(0);
                break;
            default:
                break;
        }
    }


}

static void mng_usage() {

}

static unsigned short port(const char* s) {
    char* end = 0;
    const long sl = strtol(s, &end, 10);

    if (end == s || '\0' != *end ||
        ((LONG_MIN == sl || LONG_MAX == sl) && ERANGE == errno) || sl < 0 ||
        sl > USHRT_MAX) {
        log_print(LOG_ERROR, "port should in in the range of 1-65536: %s\n", s);
        return 1;
    }
    return (unsigned short) sl;
}