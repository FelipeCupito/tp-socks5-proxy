#include "../include/client_handler.h"

void client_passive_accept(struct selector_key *key) {
    
    struct sockaddr_storage clntAddr; // Client address
    socklen_t clntAddrLen = sizeof(clntAddr);// Set length of client address structure (in-out parameter)

    // Wait for a client to connect
    const int client = accept(key->fd, (struct sockaddr*) &clntAddr, &clntAddrLen);
    if (client == -1) {
        goto fail;
    }
    printf("acepto al conecion(1)\n");


    // clntSock is connected to a client!
    if (selector_fd_set_nio(client) == -1) {
        goto fail;
    }

    if (SELECTOR_SUCCESS != selector_register(key->s, client, &client_handler, OP_READ, NULL)) {
        goto fail;
    }
   printf("entro al select(1)\n");
    
fail:
    if (client != -1) {
        //printf("no\n");
        //close(client);
    }
    //close(key->fd);
}



void client_read(struct selector_key *key){
    printf("llamo a la funcion de read(23)\n");
    printf("%d\n", key->data);

}
void client_write(struct selector_key *key){
     printf("llamo a la funcion de read(2)\n");

}
void client_block(struct selector_key *key){
     printf("llamo a la funcion de read(1)\n");

}
void client_close(struct selector_key *key){
    close(key->fd);
    
}

