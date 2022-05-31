#include "../include/ipversion.h"

int ip_version(const char *src) {
    char buf[16];
    if (inet_pton(AF_INET, src, buf)) {
        return 4;
    } else if (inet_pton(AF_INET6, src, buf)) {
        return 6;
    }
    return -1;
}

// int main(int argc, char *argv[]) {
//     for (int i = 1; i < argc; ++i) {
//         printf("%s\t%d\n", argv[i], ip_version(argv[i]));
//     }

//     return 0;
// }