#include "include/management_protocol_handler.h"

#define GETOPTSIZE 9

// Funciones privadas
static void send_receive_delete(int fd, char* username);
static uint8_t send_delete_request(int fd, char* username);
static uint8_t receive_delete_reply(int fd, uint8_t* status);

static uint8_t* send_receive_get(int fd, uint8_t command);
static int send_get_request(int fd, uint8_t command);
static uint8_t* receive_get_request(int fd, uint8_t* status);

static void send_receive_put(int fd, char* username, char* password);
static int send_put_request(int fd, char* username, char* password);
static int receive_put_reply(int fd, uint8_t* status);

static void send_receive_edit(int fd, char* username, uint8_t attribute, char* value);
static int send_edit_request(int fd, char* username, uint8_t attribute, char* value);
static uint8_t receive_edit_reply(int fd, uint8_t* status);

static void send_receive_configbuffsize(int fd, unsigned int size);
static int send_configbuffsize_request(int fd, unsigned int size);
static uint8_t receive_configbuffsize_reply(int fd, uint8_t* status);

static void send_receive_configstatus(int fd, uint8_t field, uint8_t status);
static int send_configstatus_request(int fd, uint8_t field, uint8_t status);
static uint8_t receive_configstatus_reply(int fd, uint8_t* status);

static void print_get_response(int status);
static void print_put_response(int status);
static void print_edit_response(int status);
static void print_configstatus_response(int status);
static void print_configbuffsize_response(int status);
static void print_delete_response(int status);

static int hash_get(char* get_option);


static void getUsers(int fd);
static void getPasswords(int fd);
static void getBufferSize(int fd);
static void getAuthStatus(int fd);
static void getSpoofStatus(int fd);
static void getSentBytes(int fd);
static void getReceivedBytes(int fd);
static void getBytes(int fd, uint8_t command, char* msg);
static void getHistoricalConnections(int fd);
static void getConcurrentConections(int fd);
static void getConnections(int fd, uint8_t command, char* msg);
static void getStatus(int fd, uint8_t command, char* msg);

static void addUser(int fd, char* username, char* password);
static void deleteUser(int fd, char* username);
static void setBufferSize(int fd, unsigned int size);

static void editUsername(int fd, char* username, char* newUsername);
static void editPassword(int fd, char* username, char* newPassword);
static void edit(int fd, char* username, char* newField, uint8_t command, char* msg);

static void configAuth(int fd, uint8_t status);
static void configSpoof(int fd, uint8_t status);
static void configStatus(int fd, uint8_t status, uint8_t command, char* msg);

char* get_cmds[] = {"users", "passwords", "buffersize", "authstatus", "spoofingstatus", "sentbytes", "receivedbytes", "historic", "concurrent"};

char* get_msg[] = { "Success", "Invalid action", "Invalid option" };
char* put_msg[] = { "Success", "Invalid action", "Invalid username length", "Invalid password length" };
char* edit_msg[] = { "Success", "Invalid action", "Invalid field", "Invalid username length", "Invalid attribute", "Invalid value length" };
// TODO: Preguntar que es invalid field en delete
char* delete_msg[] = { "Success", "Invalid action", "Invalid field", "Invalid username length", "Unknown user fail" };
char* configstatus_msg[] = { "Success", "Invalid action", "Invalid field", "Invalid status" };
char* configbuffsize_msg[] = { "Success", "Invalid action", "Invalid buffer size length", "Invalid buffer size" };

enum get_options {
    USERS,
    PASSWORDS,
    BUFFERSIZE,
    AUTH_STATUS,
    SPOOFING_STATUS,
    SENT_BYTES,
    RECEIVED_BYTES,
    HISTORIC_CONNECTIONS,
    CONCURRENT_CONNECTIONS,
} get_options;

void login(int fd, struct manage_args* args) {
    char* password = args->try_password;

     //Creamos primer mensaje
    size_t password_len = strlen(password);

    if(password_len > 255) {
        fprintf(stderr, "[AUTH] Password too long\n");
        exit(1);
    }

    // TODO: Fijarse que hay un warning de que no esta inicializado
    // hacer uint8_t*msg = NULL;
    uint8_t* msg = realloc(msg, 2 + password_len + 2);
    msg[0] = 0x00;
    msg[1] = password_len;
    strcpy((char*) (msg + 2), password);

    printf("Code %u\n", msg[0]);
    printf("pass len: %u\n", msg[1]);
    printf("About to send auth with %s\n", (char*) (msg + 2));
    
    // using send due to connected state
    int bytes = send(fd, msg, password_len+4, 0);   // MSG_NOSIGNAL -> don't generate a SIGPIPE
    printf("bytes sent: %d\n", bytes);
  

    // recibir respuesta
    char res[1];
    recv(fd, res, 1, 0);

    switch (res[0]) {
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
    if (args->add_flag && args->add_username != NULL && args->add_password != NULL) {
        addUser(fd, args->add_username, args->add_password);
    }
    if (args->set_flag) {
        setBufferSize(fd, args->set_size);
    }
    if (args->delete_flag && args->delete_username != NULL) {
        deleteUser(fd, args->delete_username);
    }
    if(args->edit_flag && args->edit_username != NULL && args->edit_value != NULL) {
        if(args->edit_attribute == 0) {
            editUsername(fd, args->edit_username, args->edit_value);
        }
        if(args->edit_attribute == 1) {
            editPassword(fd, args->edit_username, args-> edit_value);
        }
    }
    if(args->toggle_flag) {
        if(strcmp("auth", args->toggle_option) == 0) {
            if(strcmp("on", args->toggle_status) == 0) 
                configAuth(fd, 0x00);
            if(strcmp("off", args->toggle_status) == 0)
                configAuth(fd, 0x01);
        } else if(strcmp("spoof", args->toggle_option) == 0) {
            if(strcmp("on", args->toggle_status) == 0) 
                configSpoof(fd, 0x00);
            if(strcmp("off", args->toggle_status) == 0)
                configSpoof(fd, 0x01);
        }
    }
    if (args->get_flag) {
        switch (hash_get(args->get_option)) {
            case USERS:
                getUsers(fd);
                break;
            case PASSWORDS:
                getPasswords(fd);
                break;
            case BUFFERSIZE:
                getBufferSize(fd);
                break;
            case AUTH_STATUS:
                getAuthStatus(fd);
                break;
            case SPOOFING_STATUS:
                getSpoofStatus(fd);
                break;
            case SENT_BYTES:
                getSentBytes(fd);
                break;
            case RECEIVED_BYTES:
                getReceivedBytes(fd);
                break;
            case HISTORIC_CONNECTIONS:
                getHistoricalConnections(fd);
                break;
            case CONCURRENT_CONNECTIONS:
                getConcurrentConections(fd);
                break;
            default:
                break;
        }
    }
}

// GET ACTION handlers
static void getUsers(int fd) {
    uint8_t* users_list = send_receive_get(fd, 0x00);

    if (users_list == NULL) {
        return; // Todo el chequeo de errores ya esta implementado
    }

    printf("USER LIST\n\n");
    printf("%s", (char*) users_list);

    // TODO: preguntar si es valido asi
    free(users_list);
}

static void getPasswords(int fd) {
    uint8_t* passwords_list = send_receive_get(fd, 0x01);

    if (passwords_list == NULL) {
        return; // Todo el chequeo de errores ya esta implementado
    }

    printf("PASSWORD LIST\n\n");
    printf("%s", (char*) passwords_list);

    free(passwords_list);
}

static void getHistoricalConnections(int fd) {
    getConnections(fd, 0x07, "HISTORICAL CONNECTIONS");
}

static void getConcurrentConections(int fd) {
    getConnections(fd, 0x08, "CURRENT CONNECTIONS");
}

// TODO: Chequear si esta bien el shifting
static void getConnections(int fd, uint8_t command, char* msg) {
    uint8_t* reply = send_receive_get(fd, command);

    if (reply == NULL) {
        return;     // errores ya manejados
    }

    unsigned int bytes = reply[3] | (reply[2] << 8) | (reply[1] << 16) | (reply[0] << 24);

    printf("%s: %u", msg, bytes);

    free(reply);
}

// TODO: Chequear si esta bien el shifting
static void getBufferSize(int fd) {
    uint8_t* reply = send_receive_get(fd, 0x02);

    if(reply == NULL) {
        return;
    }

    unsigned int buffer_size = reply[3] | (reply[2] << 8) | (reply[1] << 16) | (reply[0] << 24);

    printf("Buffer size: %u", buffer_size);

    free(reply);
}

static void getAuthStatus(int fd) {
    getStatus(fd, 0x03, "Auth status");
}

// TODO: Chequear que respuesta recibe
static void getSpoofStatus(int fd) {
    getStatus(fd, 0x04, "Spoof status");
}

static void getStatus(int fd, uint8_t command, char* msg) {
    uint8_t* reply = send_receive_get(fd, command);

    if(reply == NULL) {
        return;
    }

    char* status = (reply[0] == 0x00) ? "on" : "off";

    printf("%s: %s\n", msg, status);
}

// TODO: Chequear si esta bien el shifting
static void getSentBytes(int fd) {
    getBytes(fd, 0x05, "Sent Bytes");
}

static void getReceivedBytes(int fd) {
    getBytes(fd, 0x06, "Received Bytes");
}

static void getBytes(int fd, uint8_t command, char* msg) {
    uint8_t* reply = send_receive_get(fd, command);

    if(reply == NULL) {
        return;
    }

    uint32_t reply_bytes = reply[3] | (reply[2] << 8) | (reply[1] << 16) | (reply[0] << 24);

    printf("%s: %u\n", msg, reply_bytes);

    free(reply);
}

// PUT ACTION handlers
static void addUser(int fd, char* username, char* password) {
    printf("ADD USER ACTION REQUESTED\n");
    send_receive_put(fd, username, password);
}

// DELETE ACTION handlers
static void deleteUser(int fd, char* username) {
    printf("DELETE USER ACTION REQUESTED\n");
    send_receive_delete(fd, username);
}

// CONFIGBUFFSIZE ACTION handlers
static void setBufferSize(int fd, unsigned int size) {
    send_receive_configbuffsize(fd, size);
    printf("New buffer size set to: %d", size);
}

// EDIT ACTION handlers
static void editUsername(int fd, char* username, char* newUsername) {
    edit(fd, username, newUsername, 0x00, "Username changed to");
}

static void editPassword(int fd, char* username, char* newPassword) {
    edit(fd, username, newPassword, 0x01, "Password changed to");
}

static void edit(int fd, char* username, char* newField, uint8_t command, char* msg) {
    send_receive_edit(fd, username, command, newField);
    printf("%s: %s", msg, newField);
}

// CONFIGSTATUS ACTION handlers
static void configAuth(int fd, uint8_t status) {
    configStatus(fd, status, 0x03, "Auth status changed to");
}

static void configSpoof(int fd, uint8_t status) {
    configStatus(fd, status, 0x04, "Spoof status changed to");
}

static void configStatus(int fd, uint8_t status, uint8_t command, char* msg) {
    send_receive_configstatus(fd, command, status);
    printf("%s: %s", msg, (status == 0)? "on" : "off");
}

// Requests and replies

/* ------------------------------------------------------ */
/*                         DELETE                         */
/* ------------------------------------------------------ */

static void send_receive_delete(int fd, char* username) {
    if (send_delete_request(fd, username) <= 0) {
        printf("[DELETE] Error in sending request\n");
        perror(strerror(errno));
        exit(1);
    }

    uint8_t status;
    int recv_bytes = receive_delete_reply(fd, &status);

    if (recv_bytes <= 0) {
        perror(strerror(errno));
        printf("[DELETE] Server error\n");
    }
    else {
        print_edit_response(status);
    }
}

static uint8_t send_delete_request(int fd, char* username) {
    // TODO preguntar
    int sent_bytes;
    size_t username_len = strlen(username);
    uint8_t* request = NULL;

    request = realloc(request, 3 + username_len + 1);
    request[0] = 0x05;
    request[1] = 0x00;
    request[2] = username_len;
    strcpy((char*) (request + 3), username);

    sent_bytes = send(fd, request, username_len + 3, MSG_NOSIGNAL);

    free(request);
    return sent_bytes;
}

static uint8_t receive_delete_reply(int fd, uint8_t* status) {
    int rcv_bytes;
    uint8_t reply;
    rcv_bytes = recv(fd, &reply, 1, 0);

    *status = reply;
    return rcv_bytes;
}

/* ------------------------------------------------------ */
/*                           GET                          */
/* ------------------------------------------------------ */

static uint8_t* send_receive_get(int fd, uint8_t command) {
    int sent_bytes = send_get_request(fd, command);

    if (sent_bytes <= 0) {
        perror(strerror(errno));
        printf("[GET] Server error\n");
        exit(1);
        return NULL;
    }
    uint8_t status;
    uint8_t* reply = receive_get_request(fd, &status);

    if (reply == NULL) {
        if (status != STATUS_OK) {
            print_get_response(status);
        }
        else {
            perror(strerror(errno));
            printf("[GET] Server error\n");
        }
    }

    return reply;
}

static int send_get_request(int fd, uint8_t command) {
    int sent_bytes = 0;

    uint8_t request[2];

    request[0] = 0x00;      // Action GET
    request[1] = command;   // Command passed

    // Return number of sent bytes / -1 if error
    sent_bytes = send(fd, request, 2, 0);

    return sent_bytes;
}

static uint8_t* receive_get_request(int fd, uint8_t* status) {
    int recv_bytes;
    uint8_t info[2];

    recv_bytes = recv(fd, info, 2, 0);

    if (recv_bytes <= 0) {
        exit(1);
    }

    if (recv_bytes < 2) {
        *status = SERVER_ERROR;
        return NULL;
    }

    *status = info[0];
    // Solo nos interesa guardar la respuesta con STATUS_OK
    if (*status != STATUS_OK) {
        return NULL;
    }

    size_t rta_len = info[1];
    uint8_t* rta = malloc(rta_len + 1);

    // Chequear si hay espacio suficiente
    if (rta == NULL) {
        perror(strerror(errno));
        printf("[GET] Not enough space for buffer");
        return NULL;
    }

    recv(fd, rta, rta_len + 1, 0);

    rta[rta_len] = '\0';

    return rta;
}

/* ------------------------------------------------------ */
/*                           PUT                          */
/* ------------------------------------------------------ */

static void send_receive_put(int fd, char* username, char* password) {
    int sent_bytes = send_put_request(fd, username, password);

    // TODO: Chequear manejo de errores
    if (sent_bytes <= 0) {
        perror(strerror(errno));
        printf("[PUT] Error in sending request\n");
        exit(1);
    }

    uint8_t status;
    int rcv_bytes = receive_put_reply(fd, &status);

    if (rcv_bytes <= 0) {
        perror(strerror(errno));
        printf("[PUT] Server error\n");
        exit(1);
    }
    else {
        print_put_response(status);
    }
}

static int send_put_request(int fd, char* username, char* password) {
    int sent_bytes = 0;
    uint8_t* request = NULL;
    size_t username_len = strlen(username);
    size_t password_len = strlen(password);

    request = realloc(request, 2 + 2 * sizeof(int) + username_len + password_len + 1);
    request[0] = 0x01;
    request[1] = 0x00;
    request[2] = username_len;
    strcpy((char*) (request + 3), username);
    request[3 + username_len] = password_len;
    strcpy((char*) (request + 4 + username_len), password);

    sent_bytes = send(fd, request, username_len + password_len + 4, MSG_NOSIGNAL);

    free(request);

    return sent_bytes;
}

static int receive_put_reply(int fd, uint8_t* status) {
    int rcv_bytes;
    uint8_t reply[1];
    rcv_bytes = recv(fd, reply, 1, 0);

    return rcv_bytes;
}

/* ------------------------------------------------------ */
/*                         EDIT                           */
/* ------------------------------------------------------ */

static void send_receive_edit(int fd, char* username, uint8_t attribute, char* value) {
    if (send_edit_request(fd, username, attribute, value) <= 0) {
        printf("[EDIT] Error in sending the request\n");
        perror(strerror(errno));
        exit(1);
    }

    uint8_t status;
    int rcv_bytes = receive_edit_reply(fd, &status);

    if (rcv_bytes <= 0) {
        perror(strerror(errno));
        printf("[EDIT] Server error\n");
    }
    else {
        print_edit_response(status);
    }
}

static int send_edit_request(int fd, char* username, uint8_t attribute, char* value) {
    int sent_bytes;

    uint8_t* request = NULL;
    size_t username_len = strlen(username);
    size_t value_len = strlen(value);

    request = realloc(request, 5 + username_len + value_len + 1);
    request[0] = 0x02;
    request[1] = 0x00;
    request[2] = username_len;
    strcpy((char*) (request + 3), username);
    request[3 + username_len] = attribute;
    request[4 + username_len] = value_len;
    strcpy((char*) (request + 5 + username_len), value);

    sent_bytes = send(fd, request, username_len + value_len + 6, MSG_NOSIGNAL);

    // TODO: ver cuando hacer el free
    free(request);

    return sent_bytes;
}

static uint8_t receive_edit_reply(int fd, uint8_t* status) {
    int rcv_bytes;
    uint8_t reply[1];
    rcv_bytes = recv(fd, reply, 1, 0);

    *status = reply[0];

    return rcv_bytes;
}

/* ------------------------------------------------------ */
/*                    CONFIGBUFFSIZE                      */
/* ------------------------------------------------------ */

static void send_receive_configbuffsize(int fd, unsigned int size) {
    if (send_configbuffsize_request(fd, size) <= 0) {
        printf("[CONFIGBUFFSIZE] Error in sending request\n");
        perror(strerror(errno));
        exit(1);
    }

    uint8_t status;
    int recv_bytes = receive_configbuffsize_reply(fd, &status);

    if (recv_bytes <= 0) {
        if (recv_bytes < 0) {
            // Negativo -> error
            perror(strerror(errno));

        }
        printf("[CONFIGBUFFSIZE] Server error\n");
    }
    else {
        print_configbuffsize_response(status);
    }
}

static int send_configbuffsize_request(int fd, unsigned int size) {
    int sent_bytes = 0;
    uint8_t request[2];

    request[0] = 0x03;
    request[1] = size;

    sent_bytes = send(fd, request, 2, MSG_NOSIGNAL);

    free(request);
    return sent_bytes;
}

static uint8_t receive_configbuffsize_reply(int fd, uint8_t* status) {
    int rcv_bytes;
    uint8_t reply[1];
    rcv_bytes = recv(fd, reply, 1, 0);

    status = reply;

    return rcv_bytes;
}

/* ------------------------------------------------------ */
/*                     CONFIGSTATUS                       */
/* ------------------------------------------------------ */

// TOGGLE 
static void send_receive_configstatus(int fd, uint8_t field, uint8_t status) {

    if (send_configstatus_request(fd, field, status) <= 0) {
        perror(strerror(errno));
        exit(1);
    }

    uint8_t reply_status;
    int recv_bytes = receive_configstatus_reply(fd, &reply_status);

    // Check if error
    if (recv_bytes <= 0) {
        if (recv_bytes < 0) {
            // Negativo -> error
            perror(strerror(errno));
        }
        printf("[CONFIGSTATUS] Server error\n");
    }
    else {
        print_configstatus_response(status);
    }
}

// TOGGLE
static int send_configstatus_request(int fd, uint8_t field, uint8_t status) {
    int sent_bytes;
    uint8_t request[3];

    request[0] = 0x04;
    request[1] = field;
    request[2] = status;

    sent_bytes = send(fd, request, 3, 0);

    if (sent_bytes) {
        // server error?
    }

    return sent_bytes;
}

// TOGGLE
static uint8_t receive_configstatus_reply(int fd, uint8_t* status) {
    int rcv_bytes;
    uint8_t reply;
    rcv_bytes = recv(fd, &reply, 1, 0);

    *status = reply;
    return rcv_bytes;
}

/* ------------------------------------------------------ */
/*                   REPLY STATUS MESSAGE                 */
/* ------------------------------------------------------ */

static void print_get_response(int status) {
    if (status >= 0 && status < GET_MSG_SIZE) {
        printf("Response: [GET] %s\n", get_msg[status]);
    }
    else {
        printf("[GET] Unknown status\n");
    }
}

static void print_put_response(int status) {
    if (status >= 0 && status < PUT_MSG_SIZE) {
        printf("Response: [PUT] %s\n", put_msg[status]);
    }
    else {
        printf("[PUT] Unknown status\n");
    }
}

static void print_edit_response(int status) {
    if (status >= 0 && status < EDIT_MSG_SIZE) {
        printf("Response: [EDIT] %s\n", edit_msg[status]);
    }
    else {
        printf("[EDIT] Unknown status\n");
    }
}

static void print_configstatus_response(int status) {
    if (status >= 0 && status < CONFIGSTATUS_MSG_SIZE) {
        printf("Response: [CONFIGSTATUS] %s\n", configstatus_msg[status]);
    }
    else {
        printf("[CONFIGSTATUS] Unknown status\n");
    }
}

static void print_configbuffsize_response(int status) {
    if (status >= 0 && status < CONFIGBUFFSIZE_MGS_SIZE) {
        printf("Response: [CONFIGBUFFSIZE] %s\n", configbuffsize_msg[status]);
    }
    else {
        printf("[CONFIGSTATUS] Unknown status\n");
    }
}

static void print_delete_response(int status) {
    if (status >= 0 && status < DELETE_MSG_SIZE) {
        printf("Response: [DELETE] %s\n", delete_msg[status]);
    }
    else {
        printf("[DELETE] Unknown status\n");
    }
}

static int hash_get(char* get_option) {
    for(int i = 0; i < GETOPTSIZE; i++) {
        if(strcmp(get_option, get_cmds[i]) == 0)
            return i;
    }

    return -1;
}