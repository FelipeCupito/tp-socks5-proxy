#ifndef METRICS_H
#define METRICS_H

#include <errno.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdio.h>
#include <stdlib.h>     // calloc
#include <string.h>     // memcpy
#include <sys/socket.h> // socket
#include <sys/types.h>  // socket
#include <unistd.h>

#include "../include/logger.h"


//INIT
void init_metrics();

// SETTERS
void add_connection();
void end_connection();
void add_sent_byte(int n);
void add_received_bytes(int n);

// GETTERS
int get_current_conn();
int get_histori_conn();
uint64_t get_sent_byte();
uint64_t get_received_bytes();
uint64_t get_transfered_bytes();

#endif
