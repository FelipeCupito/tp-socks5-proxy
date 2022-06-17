#include "../../include/management_protocol/management_protocol_handler.h"

// Funciones privadas
static int send_get_request(int fd, uint8_t command);

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
        switch (args->get_option)
        {
        case USERS:
            getUsers(fd);
            break;
        
        default:
            break;
        }
    }
    if(args->get_flag) {
        
    }
}

static void getUsers(int fd) {
    int sent_bytes = send_get_request(fd, 0x00);
    
    if(sent_bytes <= 0)
        fprintf(stderr, "[ERROR] REQUEST NOT SENT\n");
    
}

void getHistoricalConnections(int fd) {

}

void getConcurrentConections(int fd) {

}

void getSentBytes(int fd) {

}

void getReceivedBytes(int fd) {
    
}

static int send_get_request(int fd, uint8_t command) {
    int sent_bytes = 0;
    enum get_status status;
    if(command == NULL) {
        fprintf(stderr, "[ERROR] Missing GET argument\n");
        exit(0);    // cambiar esto para que sea como un goto finally
    }

    char* request = malloc(2 * sizeof(uint8_t));

    request[0] = 0x00;      // Action GET
    request[1] = command;   // Command passed

    // Return number of sent bytes / -1 if error
    sent_bytes = send(fd, request, 2, 0);

    free(request);
    return sent_bytes;
}

// TODO: Fijarse cual va a ser el tamaño máximo de los buffers
static uint8_t** receive_get_request(int fd, enum get_status status) {
    enum get_status status;

    uint8_t response[GET_RESPONSE_SIZE];
    int bytes_received = recv(fd, response, GET_RESPONSE_SIZE, 0);

    if(bytes_received < 0) {
        status = GET_STATUS_SERVER_ERROR;
        return NULL;
    }
    status = response[0];
    if(status != GET_STATUS_OK) {
        return NULL;
    }
    // se deberia chequear si es que se recibio todo (preguntar)
}