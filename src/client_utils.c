#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "protocol/network.h"
#include "protocol/protocol.h"
#include "../config/client_config.h"

int send_login_request(int client_fd, const char *username, const char *password) {
    Message msg = {LOGIN, "", "", ""};
    strncpy(msg.username, username, MAX_USERNAME);
    strncpy(msg.data, password, BUFFER_SIZE);

    send(client_fd, &msg, sizeof(Message), 0);

    Message response;
    recv(client_fd, &response, sizeof(Message), 0);

    if (response.type == LOGIN_SUCCESS) {
        printf("Login successful! Welcome, %s.\n", username);
        return 1;
    } else if (response.type == LOGIN_FAILURE) {
        printf("Login failed! Please try again.\n");
        return 0;
    } else {
        printf("Unexpected response type: %d\n", response.type);
        return 0;
    }
}   