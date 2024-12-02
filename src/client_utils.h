#ifndef CLIENTUTILS_H
#define CLIENTUTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "protocol/network.c"
#include "protocol/protocol.h"
#include "../config/client_config.h"

int send_login_request(int client_fd, const char *username, const char *password);

#endif