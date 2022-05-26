#include "../include/socks5.h"

// static void socksv5_done(struct selector_key *key) {
//     const int fds[] = {
//         ATTACHMENT(key)->client_fd,
//         ATTACHMENT(key)->origin_fd,
//     };

//     for (unsigned i = 0; i < N(fds); i++) {
//         if (fds[i] != -1) {
//             if (SELECTOR_SUCCESS != selector_unregister_fd(key->s, fds[i])) {
//                 abort();
//             }
//             close(fds[i]);
//         }
//     }

//     if(get_concurrent_conections() > 0) {
//         set_concurrent_conections(get_concurrent_conections() - 1);
//     }
// }

// static void socksv5_read(struct selector_key *key) {
//     struct state_machine *stm = &ATTACHMENT(key)->stm;
//     const enum socks_v5state st = stm_handler_read(stm, key);

//     if (ERROR == st || DONE == st) {
//         socksv5_done(key);
//     }
// }

/* handler del socket pasivo que atiende conexiones  */

void socksv5_passive_accept(struct selector_key *key) {
    
    struct sockaddr_storage clntAddr; // Client address
    socklen_t clntAddrLen = sizeof(clntAddr);// Set length of client address structure (in-out parameter)
    
    //struct socks5 *state = NULL;

    // Wait for a client to connect
    const int client = accept(key->fd, (struct sockaddr) &clntAddr, &clntAddrLen);
    if (client == -1) {
        goto fail;
    }

    // clntSock is connected to a client!
    if (selector_fd_set_nio(client) == -1) {
        goto fail;
    }


    /*
    state = socks5_new(client);
    if (state == NULL) {
        goto fail;
    }
    */

    memcpy(&state->client_addr, &client_addr, client_addr_len);
    state->client_addr_len = client_addr_len;
    state->socks_info.client_addr = client_addr;
    if (SELECTOR_SUCCESS != selector_register(key->s, client, &socks5_handler, OP_READ, state)) {
        goto fail;
    }
    set_concurrent_conections(get_concurrent_conections() + 1);
    return;
    
fail:
    if (client != -1) {
        close(client);
    }
    socksv5_close(key);
}



