#include "../../include/management_protocol/management_protocol_handler.h"

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

static uint8_t send_receive_configstatus(int fd, uint8_t field, uint8_t status);
static int send_configstatus_request(int fd, uint8_t field, uint8_t status);
static uint8_t receive_configstatus_reply(int fd, uint8_t* status);

static void print_get_response(int status);
static void print_put_response(int status);
static void print_edit_response(int status);
static void print_configstatus_response(int status);
static void print_configbuffsize_response(int status);
static void print_delete_response(int status);


static void getUsers(int fd);
static void getPasswords(int fd);

bool return_main = false;

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
            getPasswords(fd);
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
            //getSentBytes(fd);
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
        //setBufferSize(fd, args->set_size);
    }
}

static void getUsers(int fd) {
    uint8_t* users_list = send_receive_get(fd, 0x00);

    if(users_list == NULL) {
        return; // Todo el chequeo de errores ya esta implementado
    }

    printf("USER LIST\n\n");
    printf("%s",(char*) users_list);

    // TODO: preguntar si es valido asi
    free(users_list);
}

static void getPasswords(int fd) {
    uint8_t* passwords_list = send_receive_get(fd, 0x01);

    if(passwords_list == NULL) {
        return; // Todo el chequeo de errores ya esta implementado
    }

    printf("PASSWORD LIST\n\n");
    printf("%s",(char*) passwords_list);

    free(passwords_list);
}

static void getHistoricalConnections(int fd) {

}

static void getConcurrentConections(int fd) {

}

static char* getConnections(int fd, uint8_t command) {
    uint8_t* reply = send_receive_get(fd, command);

    if(reply == NULL)
        return;     // errores ya manejados
    

}

/*
void getSentBytes(int fd) {
    int sent_bytes = send_get_request(fd, 0x05);

    uint8_t status;
    uint8_t* reply = receive_get_request(fd, &status);

    if(reply == NULL) {
        if(status != STATUS_OK) {
            // printf de error (personalizar mensajes)
        } else {
            // UNKNOWN ERROR
        }
    }

}
*/
/*

void getReceivedBytes(int fd) {
    int sent_bytes = send_get_request(fd, 0x06);

    uint8_t status;
    uint8_t* reply = receive_get_request(fd, &status);

    if(reply == NULL) {
        if(status != STATUS_OK) {
            // printf de error (personalizar mensajes)
        } else {
            // UNKNOWN ERROR
        }
    }
}
*/
static void addUser(int fd, char* username, char* password) {
    printf("ADD USER ACTION REQUESTED\n");
    send_receive_put(fd, username, password);
}

static void deleteUser(int fd, char* username) {

}

static void setBufferSize(int fd, unsigned int size) {
    //int sent_bytes = send_set_request(fd, size);
}

// Requests y replies

/* ------------------------------------------------------ */ 
/*                         DELETE                         */
/* ------------------------------------------------------ */ 

static void send_receive_delete(int fd, char* username) {
    if(send_delete_request(fd, username) <= 0) {
        printf("[DELETE] Error in sending request\n");
        perror(strerror(errno));
    }

    uint8_t status;
    int recv_bytes = receive_delete_reply(fd, &status);

    if(recv_bytes <= 0) {
        perror(strerror(errno));
        printf("[DELETE] Server error\n");
    } else {
        print_edit_response(status);
    }
}

static uint8_t send_delete_request(int fd, char* username) {
    // TODO preguntar
    int sent_bytes;
    size_t username_len = strlen(username);
    uint8_t *request = NULL;
    
    realloc(request, 3 + username_len + 1);
    request[0] = 0x05;
    request[1] = 0x00;
    request[2] = username_len;
    strcpy((char*) (request + 3), username);

    sent_bytes = send(fd, request, strlen((char*) request), MSG_NOSIGNAL);

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

    if(sent_bytes <= 0) {
          perror(strerror(errno));
        printf("[GET] Server error\n");
        exit(1);
        return NULL;
    }
    uint8_t status;
    uint8_t* reply = receive_get_request(fd, &status);

    if(reply == NULL) {
        if(status != STATUS_OK) {
            print_get_response(status);
        } else {
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
    uint8_t* info[2] = malloc(2);

    recv_bytes = recv(fd, info, 2, 0);

    if(recv_bytes <= 0) {
        exit(1);
    }

    if(recv_bytes < 2) {
        *status = SERVER_ERROR;
        return NULL;
    }

    *status = info[0];
    // Solo nos interesa guardar la respuesta con STATUS_OK
    if(*status != STATUS_OK) {
        return NULL;
    }

    size_t rta_len = info[1];
    uint8_t* rta = malloc(rta_len + 1);

    // Chequear si hay espacio suficiente
    if(rta == NULL) {
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
    if(sent_bytes <= 0) {
         perror(strerror(errno));
        printf("[PUT] Error in sending request\n");
    }

    uint8_t status;
    int rcv_bytes = receive_put_reply(fd, &status);

    if(rcv_bytes <= 0) {
         perror(strerror(errno));
        printf("[PUT] Server error\n");
    } else {
        print_put_response(status);
    }
}

static int send_put_request(int fd, char* username, char* password) {
    int sent_bytes = 0;
    uint8_t *request = NULL;
    size_t username_len = strlen(username);
    size_t password_len = strlen(password);

    request = realloc(request, 2 + 2*sizeof(int) + username_len + password_len + 1);
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
    if(send_edit_request(fd, username, attribute, value) <= 0) {
        printf("[EDIT] Error in sending the request\n");
         perror(strerror(errno));
    }

    uint8_t status;
    int rcv_bytes = receive_edit_reply(fd, &status);

    if(rcv_bytes <= 0) {
         perror(strerror(errno));
        printf("[EDIT] Server error\n");
    } else {
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

    sent_bytes = send(fd, request, strlen((char*) request), MSG_NOSIGNAL);

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
    if(send_configbuffsize_request(fd, size) <= 0) {
        printf("[CONFIGBUFFSIZE] Error in sending request\n");
         perror(strerror(errno));
    }

    uint8_t status;
    int recv_bytes = receive_configbuffsize_reply(fd, &status);

    if(recv_bytes <= 0) {
        if(recv_bytes < 0) {
            // Negativo -> error
             perror(strerror(errno));
            
        }
        printf("[CONFIGBUFFSIZE] Server error\n");
    } else {
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
static uint8_t send_receive_configstatus(int fd, uint8_t field, uint8_t status) {

    if(send_configstatus_request(fd, field, status) <= 0) {
         perror(strerror(errno));
    }

    uint8_t reply_status;
    int recv_bytes = receive_configstatus_reply(fd, &reply_status);

    // Check if error
    if(recv_bytes <= 0) {
        if(recv_bytes < 0) {
            // Negativo -> error
             perror(strerror(errno));
        }
        printf("[CONFIGSTATUS] Server error\n");
    } else {
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

    if(sent_bytes) {
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
    if(status >= 0 && status < GET_MSG_SIZE) {
        printf("Response: [GET] %s\n", get_msg[status]);
    } else {
        printf("[GET] Unknown status\n");
    }
}

static void print_put_response(int status) {
    if(status >= 0 && status < PUT_MSG_SIZE) {
        printf("Response: [PUT] %s\n", put_msg[status]);
    } else {
        printf("[PUT] Unknown status\n");
    }
}

static void print_edit_response(int status) {
    if(status >= 0 && status < EDIT_MSG_SIZE) {
        printf("Response: [EDIT] %s\n", edit_msg[status]);
    } else {
        printf("[EDIT] Unknown status\n");
    }
}

static void print_configstatus_response(int status) {
    if(status >= 0 && status < CONFIGSTATUS_MSG_SIZE) {
        printf("Response: [CONFIGSTATUS] %s\n", configstatus_msg[status]);
    } else {
        printf("[CONFIGSTATUS] Unknown status\n");
    }
}

static void print_configbuffsize_response(int status) {
    if(status >= 0 && status < CONFIGBUFFSIZE_MGS_SIZE) {
        printf("Response: [CONFIGBUFFSIZE] %s\n", configbuffsize_msg[status]);
    } else {
        printf("[CONFIGSTATUS] Unknown status\n");
    }
}

static void print_delete_response(int status) {
    if(status >= 0 && status < DELETE_MSG_SIZE) {
        printf("Response: [DELETE] %s\n", delete_msg[status]);
    } else {
        printf("[DELETE] Unknown status\n");
    }
}