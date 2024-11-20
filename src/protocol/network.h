#ifndef NETWORK_H
#define NETWORK_H

#include <arpa/inet.h>

int create_server_socket();
int create_client_socket(const char *ip);

#endif
