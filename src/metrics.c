#include "../include/metrics.h"

static uint64_t sent_byte = 0;
static uint64_t received_bytes = 0;
static int historic_connections = 0;
static int current_connections = 0;
static int connecting_clients = 0;



//INIT
void init_metrics(){
    sent_byte = 0;
    received_bytes = 0;
    historic_connections = 0;
    current_connections = 0;
}

// SETTERS
void add_connecting_clients(){
  connecting_clients++;
}

void end_connecting_clients(){
  connecting_clients--;
}

void add_connection(){
    historic_connections++;
    current_connections++;
}

void end_connection(enum socks_response_status status){
    current_connections--;
    if(status != status_close){
      historic_connections--;
    }
    if(current_connections < 0)
        log_print(LOG_ERROR, "current_connections: %d es < 0", current_connections);
}

void add_sent_byte(int n){  
    sent_byte += n;
}

void add_received_bytes(int n){  
    received_bytes += n;
}

// GETTERS
int get_current_conn(){
    return current_connections;
}

int get_histori_conn(){
    return historic_connections;
}

uint64_t get_sent_byte(){
    return sent_byte;
}

uint64_t get_received_bytes(){
    return received_bytes;
}

uint64_t get_transfered_bytes(){
    return sent_byte + received_bytes;
}



