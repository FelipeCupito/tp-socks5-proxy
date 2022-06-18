#include "../../include/management_protocol/management_protocol_handler.h"

// Funciones privadas
static int send_get_request(int fd, uint8_t command);
static uint8_t receive_toggle_reply(int fd);
static void getUsers(int fd);
static void getPasswords(int fd);
// char* token = "somos_grupo_6";

void login(int fd, struct manage_args* args) {
    char * password = args->try_password;

    // Creamos primer mensaje
    size_t password_len = strlen(password);
    char* msg = malloc(password_len + 3);
    msg[0] = 0x00;
    msg[1] = password_len;
    strcpy((char *)(msg + 2), password);
    
    // using send due to connected state
    send(fd, msg, strlen(msg), MSG_NOSIGNAL);   // MSG_NOSIGNAL -> don't generate a SIGPIPE

    // recibir respuesta
    char res[1];
    recv(fd, res, 1, 0);

    switch(res[0]) {
        case CONN_STATUS_OK:
            printf("[INFO] AUTHORIZED\n");
            args->authorized = true;
            break;
        case CONN_STATUS_ERROR_IN_VERSION:
            fprintf(stderr, "[ERROR] VERSION NOT SUPPORTED\n");
            break;
        case CONN_STATUS_ERROR_IN_PASSLEN:
            fprintf(stderr, "[ERROR] INCORRECT TOKEN\n");
            break;
        case CONN_STATUS_ERROR_IN_PASS:
            fprintf(stderr, "[ERROR] INCORRECT PASSWORD\n");
            break;
        default:
            fprintf(stderr, "[ERROR] UNKNOWN RESPONSE\n");
    }
    
    // Liberar memoria reservada para el mensaje
    free(msg);
}

void executeCommands(int fd, struct manage_args* args) {
    if(args->list_flag) {
        switch (args->list_option)
        {
        case USERS:
            getUsers(fd);
            break;
        case PASSWORDS:
            break;
        case BUFFERSIZE:
            break;
        case AUTH_STATUS:
            break;
        case SPOOFING_STATUS:
            break;
        default:
            break;
        }
    }
    if(args->get_flag) {
        switch (args->get_option)
        {
        case SENT_BYTES:
            /* code */
            break;
        case RECEIVED_BYTES: 
            break;
        case HISTORIC_CONNECTIONS:
            break;
        case CONCURRENT_CONNECTIONS:
            break;
        default:
            break;
        }
    }
    if(args->add_flag) {
        
    }
    if(args->set_flag) {
        setBufferSize(fd, args->set_size);
    }
}

static void getUsers(int fd) {
    int sent_bytes = send_get_request(fd, 0x00);
    
    if(sent_bytes <= 0)
        fprintf(stderr, "[ERROR] REQUEST NOT SENT\n");
    
}

static void getPasswords(int fd) {

}

void getHistoricalConnections(int fd) {

}

void getConcurrentConections(int fd) {

}

void getSentBytes(int fd) {

}

void getReceivedBytes(int fd) {
    
}

static void addUser(int fd, char* username, char* password) {

}

static void deleteUser(int fd, char* username) {

}

static void setBufferSize(int fd, unsigned int size) {
    int sent_bytes = send_set_request(fd, size);
}

// Requests y replies
static uint8_t send_delete_request(int fd, char* username) {
    // TODO preguntar
    int sent_bytes = 0;
    size_t username_len = strlen(username);
    uint8_t *request = NULL;
    // TODO por que 2 + ? y + 1?
    realloc(request, 2 + 2 * sizeof(int) + username_len + 1);
    request[0] = 0x05;
    request[1] = 0x00;
    request[2] = username_len;
    strcpy((char*) (request + 3), username);

    sent_bytes = send(fd, request, strlen((char*) request), 0);

    free(request);
    return sent_bytes;
}

static int send_put_request(int fd, char* username, char* password) {
    int sent_bytes = 0;
    uint8_t *request = NULL;
    size_t username_len = strlen(username);
    size_t password_len = strlen(password);

    realloc(request, 2 + 2*sizeof(int) + username_len + password_len + 1);
    request[0] = 0x01;
    request[1] = 0x00;
    request[2] = username_len;
    strcpy((char*) (request + 3), username);
    request[3 + username_len] = password_len;
    strcpy((char*) (request + 4 + username_len), password);

    sent_bytes = send(fd, request, strlen((char*) request), MSG_NOSIGNAL);

    free(request);
    return sent_bytes;
}

static uint8_t receive_delete_reply(int fd) {
    int rcv_bytes;
    uint8_t reply;
    rcv_bytes = recv(fd, &reply, 1, 0);

    return reply;
}

static int send_get_request(int fd, uint8_t command) {
    int sent_bytes = 0;
    enum get_status status;
    if(command == NULL) {
        fprintf(stderr, "[ERROR] Missing GET argument\n");
        exit(0);    // cambiar esto para que sea como un goto finally
    }

    uint8_t* request = malloc(2 * sizeof(uint8_t));

    request[0] = 0x00;      // Action GET
    request[1] = command;   // Command passed

    // Return number of sent bytes / -1 if error
    sent_bytes = send(fd, request, 2, 0);

    free(request);
    return sent_bytes;
}

// TODO: Fijarse cual va a ser el tamaño máximo de los buffers
static uint8_t** receive_get_request(int fd, enum get_status* status) {
    int recv_bytes;
    uint8_t* info[2];

    recv_bytes = recv(fd, info, 2, 0);

    if(recv_bytes < 2) {
        // *status = TODO: codigo de error
    }

    *status = info[0];
    // Solo nos interesa guardar la respuesta con STATUS_OK
    if(*status != STATUS_OK) {
        return NULL;
    }

    size_t rta_len = info[1];
    uint8_t* rta = malloc(rta_len);

    // Chequear si hay espacio suficiente
    if(rta == NULL) {
        return NULL;
    }

    

    enum get_status status;

    // uint8_t response[GET_RESPONSE_SIZE];
    // int bytes_received = recv(fd, response, GET_RESPONSE_SIZE, 0);

    // if(bytes_received < 0) {
    //     status = GET_STATUS_SERVER_ERROR;
    //     return NULL;
    // }
    // status = response[0];
    // if(status != GET_STATUS_OK) {
    //     return NULL;
    // }
    // se deberia chequear si es que se recibio todo (preguntar)
}

static int send_set_request(int fd, unsigned int size) {
    int sent_bytes = 0;
    uint8_t request[3];

    request[0] = 0x03;
}

static enum set_status receive_set_request(int fd) {

}

static int send_toggle_request(int fd, uint8_t field, uint8_t status) {
    int sent_bytes;
    uint8_t request[3];

    request[0] = 0x04;
    request[1] = field;
    request[2] = status;

    sent_bytes = send(fd, request, 3, 0);

    if(sent_bytes) {
        // server error?
    }

    return sent_bytes;
}

static uint8_t receive_toggle_reply(int fd) {
    int rcv_bytes;
    uint8_t reply;
    rcv_bytes = recv(fd, &reply, 1, 0);

    return reply;
}